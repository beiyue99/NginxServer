
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
    
    ngx_log_stderr(0,"accept惊群测试！\n"); 

    socklen = sizeof(mysockaddr);
    do   //用do，跳到while后边去方便
    {     
        if(use_accept4)
        {
            s = accept4(oldc->fd, &mysockaddr, &socklen, SOCK_NONBLOCK);
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
            
            if (err == EMFILE || err == ENFILE) 
            {
                //do nothing，这个官方做法是先把读事件从listen socket上移除，然后再弄个定时器，
                // 定时器到了则继续执行该函数，但是定时器到了有个标记，会把读事件增加到listen socket上去；
                //我这里目前先不处理吧【因为上边已经写这个日志了】；
            }            
            return;
        }  

        newc = ngx_get_connection(s); 
        if(newc == NULL)
        {
            if(close(s) == -1)
            {
                ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocekt::ngx_event_accept()中close(%d)失败!",s);                
            }
            return;
        }
        //成功的拿到了连接池中的一个连接
        //...........将来这里会判断是否连接超过最大允许连接数，现在，这里可以不处理
        memcpy(&newc->s_sockaddr,&mysockaddr,socklen); 
        {
            u_char ipaddr[100]; memset(ipaddr,0,sizeof(ipaddr));
            ngx_sock_ntop(&newc->s_sockaddr,1,ipaddr,sizeof(ipaddr)-10); //宽度给小点
            ngx_log_stderr(0,"成功接入用户，ip信息为%s\n",ipaddr);
        }

        if(!use_accept4)
        {
            if(setnonblocking(s) == false)
            {
                //设置非阻塞居然失败
                ngx_close_connection(newc); //回收连接池中的连接（千万不能忘记），并关闭socket
                return; //直接返回
            }
        }

        newc->listening = oldc->listening;                
        newc->w_ready = 1;                                
        
        newc->rhandler = &CSocekt::ngx_wait_request_handler; 
        if(ngx_epoll_add_event(s,                 //socket句柄
                                1,0,              
                                0,//EPOLLET,          //其他补充标记【EPOLLET(高速模式，边缘触发ET)】
                                EPOLL_CTL_ADD,    //事件类型【增加，还有删除/修改】                                    
                                newc              //连接池中的连接
                                ) == -1)
        {
            ngx_close_connection(newc);//回收连接池中的连接（千万不能忘记），并关闭socket
            return; //直接返回
        } 

        break;  //一般就是循环一次就跳出去
    } while (1);   

    return;
}

