
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
#include <sys/ioctl.h> //ioctl
#include <arpa/inet.h>

#include "ngx_c_conf.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_socket.h"

void CSocekt::ngx_event_accept(lpngx_connection_t oldc)
{
    //因为listen套接字上用的不是ET【边缘触发】，而是LT【水平触发】，意味着客户端连入如果我要不处理，
    // 这个函数会被多次调用，所以，我这里这里可以不必多次accept()，可以只执行一次accept()
    struct sockaddr    mysockaddr;        //远端服务器的socket地址
    socklen_t          socklen;
    int                err;
    int                level;
    int                s;
    static int         use_accept4 = 1;   //我们先认为能够使用accept4()函数
    lpngx_connection_t newc;              //代表连接池中的一个连接【注意这是指针】
    
    //ngx_log_stderr(0,"惊群测试！！"); 
    socklen = sizeof(mysockaddr);
    do   
    {     
        if(use_accept4)
        {
            //listen套接字是非阻塞的，所以即便已完成连接队列为空，accept4()也不会卡在这里；
            s = accept4(oldc->fd, &mysockaddr, &socklen, SOCK_NONBLOCK); 
            //从内核获取一个用户端连接，最后一个参数SOCK_NONBLOCK表示返回一个非阻塞的socket，节省一次ioctl【设置为非阻塞】调用
        }
        else
        {
            s = accept(oldc->fd, &mysockaddr, &socklen);
        }

        if(s == -1)
        {
            err = errno;
            if(err == EAGAIN) //accept()没准备好，这个EAGAIN错误EWOULDBLOCK是一样的
            {
                return ;
            } 
            level = NGX_LOG_ALERT;
            if (err == ECONNABORTED) //服务器进程一般可以忽略该错误，直接再次调用accept。
            {
                level = NGX_LOG_ERR;
            } 
            else if (err == EMFILE || err == ENFILE) 
            {
                level = NGX_LOG_CRIT;
            }
            ngx_log_error_core(level,errno,"CSocekt::ngx_event_accept()中accept4()失败!");

            if(use_accept4 && err == ENOSYS) 
            {
                use_accept4 = 0;  
                continue;       
            }

            if (err == ECONNABORTED)  //对方关闭套接字
            {
                //do nothing
            }
            
            if (err == EMFILE || err == ENFILE) 
            {
                //do nothing
            }            
            return;
        } 

        if(m_onlineUserCount >= m_worker_connections)  
        {
            ngx_log_stderr(0,"超出系统允许的最大连入用户数(最大允许连入数%d)，关闭连入请求(%d)。",m_worker_connections,s);  
            close(s);
            return ;
        }

        ngx_log_stderr(0,"accept4成功，socket=%d",s); //s这里就是 一个句柄了
        newc = ngx_get_connection(s); 
        if(newc == NULL)
        {
            if(close(s) == -1)
            {
                ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocekt::ngx_event_accept()中close(%d)失败!",s);                
            }
            return;
        }
        memcpy(&newc->s_sockaddr,&mysockaddr,socklen); 
        {
            //测试将收到的地址弄成字符串，格式形如"192.168.1.126
            //u_char ipaddr[100]; memset(ipaddr,0,sizeof(ipaddr));
            //ngx_sock_ntop(&newc->s_sockaddr,1,ipaddr,sizeof(ipaddr)-10); //宽度给小点
            //ngx_log_stderr(0,"ip信息为%s\n",ipaddr);
        }

        if(!use_accept4)
        {
            if(setnonblocking(s) == false)
            {
                ngx_close_connection(newc);
                return; //直接返回
            }
        }

        newc->listening = oldc->listening;                    //连接对象 和监听对象关联，方便通过连接对象找监听对象【关联到监听端口】
        
        newc->rhandler = &CSocekt::ngx_read_request_handler;  //设置数据来时的读处理函数，其实官方nginx中是ngx_http_wait_request_handler()
        newc->whandler = &CSocekt::ngx_write_request_handler; //设置数据发送时的写处理函数。

         if(ngx_epoll_oper_event(
                                s,                  //socekt句柄
                                EPOLL_CTL_ADD,      //事件类型，这里是增加
                                EPOLLIN|EPOLLRDHUP, //标志，这里代表要增加的标志,EPOLLIN：可读，EPOLLRDHUP：TCP连接的远端关闭或者半关闭 ，如果边缘触发模式可以增加 EPOLLET
                                0,                  //对于事件类型为增加的，不需要这个参数
                                newc                //连接池中的连接
                                ) == -1)         
        {
            ngx_close_connection(newc);
            return; //直接返回
        }

        if(m_ifkickTimeCount == 1)
        {
            AddToTimerQueue(newc);
        }
        ++m_onlineUserCount;  //连入用户数量+1        
        break; 
    } while (1);   

    return;
}

