
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
#include <iostream>

#include "ngx_c_conf.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_socket.h"






//建立新连接
void CSocekt::ngx_event_accept(ngx_connection_sp oldc)
{
    sockaddr_storage   peer_addr{};
    socklen_t          addrlen;
    static int         use_accept4 = 1;
    for (;;)   // ✅ 用 for(;;) + EAGAIN 退出  T 模式需要循环 accept
    {
        addrlen = sizeof(peer_addr);  // 每次循环要重新设置

        int s = -1;
        if (use_accept4)
        {
            s = accept4(oldc->fd,
                reinterpret_cast<sockaddr*>(&peer_addr),
                &addrlen,
                SOCK_NONBLOCK);
        }
        else
        {
            s = accept(oldc->fd,
                reinterpret_cast<sockaddr*>(&peer_addr),
                &addrlen);
        }

        if (s == -1)
        {
            int err = errno;
            if (err == EAGAIN || err == EWOULDBLOCK)
            {
                // ✅ backlog 已经被我们 accept 干净了
                return;
            }

            int level = NGX_LOG_ALERT;
            if (err == ECONNABORTED)
                level = NGX_LOG_ERR;
            else if (err == EMFILE || err == ENFILE)
                level = NGX_LOG_CRIT;

            std::cout << "CSocekt::ngx_event_accept()中accept4()失败! 错误码: " 
                << errno << ", 原因: " << strerror(errno) << std::endl;

            if (use_accept4 && err == ENOSYS)
            {
                use_accept4 = 0;
                continue;
            }
            return;
        }

        if (m_onlineUserCount >= m_worker_connections)
        {
            std::cout << "超出系统允许的最大连入用户数(最大允许连入数: " << 
                m_worker_connections << ")，关闭连入请求(" << s << ")。" << std::endl;
            close(s);
            continue;   // ✅ 尝试继续 accept 其它连接
        }

        if (!use_accept4)
        {
            if (setnonblocking(s) == false)
            {
                close(s);
                continue;
            }
        }

        printf("新用户连接!!\n");

        auto newc = ngx_get_connection(s);
        if (!newc)
        {
            close(s);
            continue;
        }

        std::memcpy(&newc->s_sockaddr, &peer_addr, addrlen);

        newc->listening = oldc->listening;
        newc->rhandler = &CSocekt::ngx_read_request_handler;
        newc->whandler = &CSocekt::ngx_write_request_handler;

        if (ngx_epoll_oper_event(s,
            EPOLL_CTL_ADD,
            EPOLLIN | EPOLLRDHUP | EPOLLET,  // 添加 EPOLLET
            0,
            newc) == -1)
        {
            ngx_close_connection(newc);
            continue;
        }

        if (m_ifkickTimeCount == 1)
        {
            AddToTimerQueue(newc);
        }

        ++m_onlineUserCount;
        printf("用户在线数：%d\n", m_onlineUserCount.load());
    }
}

