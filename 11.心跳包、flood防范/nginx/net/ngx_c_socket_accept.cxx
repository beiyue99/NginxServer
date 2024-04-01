
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>    //uintptr_t
#include <stdarg.h>    //va_start....
#include <unistd.h>    //STDERR_FILENO等
#include <sys/time.h>  //gettimeofday
#include <time.h>      //localtime_r
#include <fcntl.h>     //open
#include <errno.h>     //errno
//#include <sys/socket.h>
#include <sys/ioctl.h> //ioctl
#include <arpa/inet.h>

#include "ngx_c_conf.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_socket.h"

//建立新连接专用函数，当新连接进入时，本函数会被ngx_epoll_process_events()所调用
void CSocket::ngx_event_accept(lpngx_connection_t oldc)
{
    struct sockaddr    mysockaddr;        //远端服务器的socket地址
    socklen_t          socklen;
    int                err;
    int                level;
    int                s;
    static int         use_accept4 = 1;   //我们先认为能够使用accept4()函数
    lpngx_connection_t newc;              //代表连接池中的一个连接【注意这是指针】
    
    //ngx_log_stderr(0,"惊群测试！！！\n");      //惊群测试



    socklen = sizeof(mysockaddr);
    do   //用do，跳到while后边去方便
    {     
        if(use_accept4)
        {
            //以为listen套接字是非阻塞的，所以即便已完成连接队列为空，accept4()也不会卡在这里；
            s = accept4(oldc->fd, &mysockaddr, &socklen, SOCK_NONBLOCK); //从内核获取一个用户端连接，最后一个参数SOCK_NONBLOCK表示返回一个非阻塞的socket，节省一次ioctl【设置为非阻塞】调用
        }
        else
        {
            //以为listen套接字是非阻塞的，所以即便已完成连接队列为空，accept()也不会卡在这里；
            s = accept(oldc->fd, &mysockaddr, &socklen);
        }

        if(s == -1)
        {
            err = errno;

            //对accept、send和recv而言，事件未发生时errno通常被设置成EAGAIN（意为“再来一次”）或者EWOULDBLOCK（意为“期待阻塞”）
            if(err == EAGAIN) //accept()没准备好，这个EAGAIN错误EWOULDBLOCK是一样的
            {
                //除非你用一个循环不断的accept()取走所有的连接，不然一般不会有这个错误【我们这里只取一个连接，也就是accept()一次】
                return ;
            } 
            level = NGX_LOG_ALERT;
            if (err == ECONNABORTED)  
            {
                level = NGX_LOG_ERR;
            } 
            else if (err == EMFILE || err == ENFILE) 
            {
                level = NGX_LOG_CRIT;
            }
            ngx_log_error_core(level,errno,"CSocket::ngx_event_accept()中accept4()失败!");

            if(use_accept4 && err == ENOSYS) //accept4()函数没实现，坑爹？
            {
                use_accept4 = 0;  //标记不使用accept4()函数，改用accept()函数
                continue;         //回去重新用accept()函数搞
            }
            if (err == ECONNABORTED)  //对方关闭套接字
            {
                //这个错误因为可以忽略，所以不用干啥
                //do nothing
            }
            
            if (err == EMFILE || err == ENFILE) 
            {
                //do nothing，这个官方做法是先把读事件从listen socket上移除，然后再弄个定时器，定时器到了则继续执行该函数，但是定时器到了有个标记，会把读事件增加到listen socket上去；
                //我这里目前先不处理吧【因为上边已经写这个日志了】；
            }            
            return;
        }  //end if(s == -1)

        //走到这里的，表示accept4()/accept()成功了        
        if(m_onlineUserCount >= m_worker_connections)  //用户连接数过多，要关闭该用户socket，因为现在也没分配连接，所以直接关闭即可
        {
            ngx_log_stderr(0,"超出系统允许的最大连入用户数(最大允许连入数%d)，关闭连入请求(%d)。",m_worker_connections,s);  
            close(s);
            return ;
        }

        newc = ngx_get_connection(s); //这是针对新连入用户的连接，和监听套接字 所对应的连接是两个不同的东西，不要搞混
        if(newc == NULL)
        {
            if(close(s) == -1)
            {
                ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocket::ngx_event_accept()中close(%d)失败!",s);                
            }
            return;
        }
        memcpy(&newc->s_sockaddr,&mysockaddr,socklen);  //拷贝客户端地址到连接对象【要转成字符串ip地址参考函数ngx_sock_ntop()】
        u_char ipaddr[100]; memset(ipaddr,0,sizeof(ipaddr));
        ngx_sock_ntop(&newc->s_sockaddr,1,ipaddr,sizeof(ipaddr)-10); //宽度给小点
        ngx_log_stderr(0,"accept成功！ip信息为%s\n",ipaddr);

        if(!use_accept4)
        {
            if(setnonblocking(s) == false)
            {
                ngx_close_connection(newc); //关闭socket,这种可以立即回收这个连接，无需延迟，因为其上还没有数据收发，谈不到业务逻辑因此无需延迟；
                return; //直接返回
            }
        }

        newc->listening = oldc->listening;                    //连接对象 和监听对象关联，方便通过连接对象找监听对象【关联到监听端口】
        
        newc->rhandler = &CSocket::ngx_read_request_handler;  //设置数据来时的读处理函数，其实官方nginx中是ngx_http_wait_request_handler()
        newc->whandler = &CSocket::ngx_write_request_handler; //设置数据发送时的写处理函数。

         if(ngx_epoll_oper_event(
                                s,                  //socekt句柄
                                EPOLL_CTL_ADD,      //事件类型，这里是增加
                                EPOLLIN|EPOLLRDHUP, //标志，这里代表要增加的标志,EPOLLIN：可读，EPOLLRDHUP：TCP连接的远端关闭或者半关闭 ，如果边缘触发模式可以增加 EPOLLET
                                0,                  //对于事件类型为增加的，不需要这个参数
                                newc                //连接池中的连接
                                ) == -1)         
        {
            ngx_close_connection(newc);//关闭socket,这种可以立即回收这个连接，无需延迟，因为其上还没有数据收发，谈不到业务逻辑因此无需延迟；
            return; //直接返回
        }

        if(m_ifkickTimeCount == 1)
        {
            AddToTimerQueue(newc);
        }
        ++m_onlineUserCount;  //连入用户数量+1        
        break;  //一般就是循环一次就跳出去
    } while (1);   

    return;
}

