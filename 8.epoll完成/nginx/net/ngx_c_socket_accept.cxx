

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

void CSocekt::ngx_event_accept(lpngx_connection_t oldc)
{
    struct sockaddr    mysockaddr;        //远端服务器的socket地址
    socklen_t          socklen;
    int                err;
    int                level;
    int                s;
    static int         use_accept4 = 1;   //我们先认为能够使用accept4()函数
    lpngx_connection_t newc;              //代表连接池中的一个连接【注意这是指针】
    
    ngx_log_stderr(0,"惊群测试2，accept新客户进程被惊动了！\n"); 

    socklen = sizeof(mysockaddr);
    do   //用do，跳到while后边去方便
    {     
        if(use_accept4)
        {
            //以为listen套接字是非阻塞的，所以即便已完成连接队列为空，accept4()也不会卡在这里；
            s = accept4(oldc->fd, &mysockaddr, &socklen, SOCK_NONBLOCK);
            //从内核获取一个用户端连接，最后一个参数SOCK_NONBLOCK表示返回一个非阻塞的socket，节省一次ioctl【设置为非阻塞】调用
        }
        else
        {
            //以为listen套接字是非阻塞的，所以即便已完成连接队列为空，accept()也不会卡在这里；
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
            if (err == ECONNABORTED)  
                //ECONNRESET错误则发生在对方意外关闭套接字后
                // 【您的主机中的软件放弃了一个已建立的连接--由于超时或者其它失败而中止接连(用户插拔网线就可能有这个错误出现)】
            {
                //该错误被描述为“software caused connection abort”，即“软件引起的连接中止”。
                level = NGX_LOG_ERR;
            } 
            else if (err == EMFILE || err == ENFILE) 
            {
                level = NGX_LOG_CRIT;
            }
            ngx_log_error_core(level,errno,"CSocekt::ngx_event_accept()中accept4()失败!");

            if(use_accept4 && err == ENOSYS) //accept4()函数没实现
            {
                use_accept4 = 0;  //标记不使用accept4()函数，改用accept()函数
                continue;         //回去重新用accept()函数搞
            }

            if (err == ECONNABORTED)  //对方关闭套接字
            {
                //do nothing
            }
            
            if (err == EMFILE || err == ENFILE)   //文件或文件描述符达到限制
            {
                //do nothing，这个官方做法是先把读事件从listen socket上移除，
                // 然后再弄个定时器，定时器到了则继续执行该函数
                //我这里目前先不处理吧【因为上边已经写这个日志了】；
            }            
            return;
        }  

        //走到这里的，表示accept4()/accept()成功了        
        ngx_log_stderr(0,"accept4接入新客户成功了！fd=%d",s); //s这里就是 一个句柄了
        newc = ngx_get_connection(s); 
        if(newc == NULL)
        {
            //连接池中连接不够用，那么就得把这个socekt直接关闭并返回了，因为在ngx_get_connection()中已经写日志了，所以这里不需要写日志了
            if(close(s) == -1)
            {
                ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocekt::ngx_event_accept()中close(%d)失败!",s);                
            }
            return;
        }
        //...........将来这里会判断是否连接超过最大允许连接数，现在，这里可以不处理

        //成功的拿到了连接池中的一个连接
        memcpy(&newc->s_sockaddr,&mysockaddr,socklen);  //拷贝客户端地址到连接对象【要转成字符串ip地址参考函数ngx_sock_ntop()】
        {
            //测试将收到的地址弄成字符串，格式形如"192.168.1.126:40904"或者"192.168.1.126"
            u_char ipaddr[100];
            memset(ipaddr,0,sizeof(ipaddr));
            ngx_sock_ntop(&newc->s_sockaddr,1,ipaddr,sizeof(ipaddr)-10); //宽度给小点
            ngx_log_stderr(0,"ip信息为%s\n",ipaddr);
        }

        if(!use_accept4)
        {
            //如果不是用accept4()取得的socket，那么就要设置为非阻塞【因为用accept4()的已经被accept4()设置为非阻塞了】
            if(setnonblocking(s) == false)
            {
                //设置非阻塞居然失败
                ngx_close_accepted_connection(newc); //回收连接池中的连接（千万不能忘记），并关闭socket
                return; //直接返回
            }
        }

        newc->listening = oldc->listening;                 
        //连接对象 和监听对象关联，方便通过连接对象找监听对象【关联到监听端口】
        newc->w_ready = 1;                                  
        //标记可以写，新连接写事件肯定是ready的；【从连接池拿出一个连接时这个连接的所有成员都是0】            
        
        newc->rhandler = &CSocekt::ngx_wait_request_handler; 
        if(ngx_epoll_add_event(s,                 //socket句柄
                                1,0,              //读，写 ,这里读为1，表示客户端应该主动给我服务器发送消息，我服务器需要首先收到客户端的消息；
                                //0,
                                EPOLLET,          //其他补充标记【EPOLLET(高速模式，边缘触发ET)】
                                EPOLL_CTL_ADD,    //事件类型【增加，还有删除/修改】                                    
                                newc              //连接池中的连接
                                ) == -1)
        {
            ngx_close_accepted_connection(newc);//回收连接池中的连接（千万不能忘记），并关闭socket
            return; //直接返回
        } 

        break;  //一般就是循环一次就跳出去
    } while (1);   

    return;
}

void CSocekt::ngx_close_accepted_connection(lpngx_connection_t c)
{
    int fd = c->fd;
    ngx_free_connection(c);
    c->fd = -1; 
    if(close(fd) == -1)
    {
        ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocekt::ngx_close_accepted_connection()中close(%d)失败!",fd);  
    }
    return;
}
