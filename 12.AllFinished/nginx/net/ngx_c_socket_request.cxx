
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
#include <pthread.h>   //多线程

#include "ngx_c_conf.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_socket.h"
#include "ngx_c_memory.h"





//调用recvproc接受数据，收完调用ngx_wait_request_handler_proc_plast入消息队列并激发线程处理
void CSocekt::ngx_read_request_handler(ngx_connection_sp pConn)
{  
    bool isflood = false; 

    // EPOLLET 模式：循环读取直到 EAGAIN
    while (true)
    {
        ssize_t reco = recvproc(pConn, pConn->precvbuf, pConn->irecvlen);
        //返回-1是断开或者错误
        if (reco <= 0)
        {
            return;
        }

        if (pConn->curStat == _PKG_HD_INIT)
        {
            if (reco == m_iLenPkgHeader)//正好收到完整包头，这里拆解包头
            {
                ngx_wait_request_handler_proc_p1(pConn, isflood);
            }
            else
            {
                pConn->curStat = _PKG_HD_RECVING;
                pConn->precvbuf = pConn->precvbuf + reco;
                pConn->irecvlen = pConn->irecvlen - reco;
            }
        }
        else if (pConn->curStat == _PKG_HD_RECVING)
        {
            if (reco == pConn->irecvlen)
            {
                ngx_wait_request_handler_proc_p1(pConn, isflood);
            }
            else
            {
                //包头还是没收完整，继续收包头
                pConn->precvbuf = pConn->precvbuf + reco;              //注意收后续包的内存往后走
                pConn->irecvlen = pConn->irecvlen - reco;              //要收的内容减少，以确保只收到完整的包头先
            }
        }
        else if (pConn->curStat == _PKG_BD_INIT)
        {
            if (reco == pConn->irecvlen)
            {
                if (m_floodAkEnable == 1)
                {
                    isflood = TestFlood(pConn);  //如果频繁，FloodAttackCount++，达到一定次数就踢出
                }
                ngx_wait_request_handler_proc_plast(pConn, isflood);
            }
            else
            {
                pConn->curStat = _PKG_BD_RECVING;
                pConn->precvbuf = pConn->precvbuf + reco;
                pConn->irecvlen = pConn->irecvlen - reco;
            }
        }
        else if (pConn->curStat == _PKG_BD_RECVING)
        {
            if (pConn->irecvlen == reco)
            {
                //包体收完整了
                if (m_floodAkEnable == 1)
                {
                    //Flood攻击检测是否开启
                    isflood = TestFlood(pConn);
                }
                ngx_wait_request_handler_proc_plast(pConn, isflood);
            }
            else
            {
                //包体没收完整，继续收
                pConn->precvbuf = pConn->precvbuf + reco;
                pConn->irecvlen = pConn->irecvlen - reco;
            }
        }

        if (isflood == true)
        {
            ngx_log_stderr(errno, "发现客户端flood，踢掉该客户端!");
            zdClosesocketProc(pConn);
            return;
        }
    }
}

ssize_t CSocekt::recvproc(ngx_connection_sp pConn, char* buff, ssize_t buflen)
{
    ssize_t n = recv(pConn->fd, buff, buflen, 0);

    if (n == 0)
    {
        // 客户端关闭连接
        ngx_log_stderr(0, "连接被客户端正常关闭[4路挥手关闭]!");
        zdClosesocketProc(pConn);
        return -1;
    }

    if (n < 0)
    {
        // ✅ EAGAIN 是正常的，表示数据读完了
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // 不要打印日志，这是正常情况
            return -1;
        }

        if (errno == EINTR)
        {
            // 被信号中断，可以重试
            ngx_log_stderr(errno, "recv() 被信号中断");
            return -1;
        }

        if (errno == ECONNRESET)
        {
            // 连接被重置
            ngx_log_stderr(errno, "recv() 连接被重置");
        }
        else
        {
            ngx_log_stderr(errno, "recv() 发生错误");
        }

        ngx_log_stderr(0, "连接被客户端非正常关闭!");
        zdClosesocketProc(pConn);
        return -1;
    }

    return n;
}


//拆解包头，new出收pTmpBuffer指针，指向消息头+包头+包体大小的内存，拷贝包头进来，然后收包体
void CSocekt::ngx_wait_request_handler_proc_p1(ngx_connection_sp pConn, bool& isflood)
{
    CMemory& memory = CMemory::GetInstance();
    LPCOMM_PKG_HEADER pPkgHeader = reinterpret_cast<LPCOMM_PKG_HEADER>(pConn->dataHeadInfo);
    unsigned short e_pkgLen = ntohs(pPkgHeader->pkgLen);

    if (e_pkgLen < m_iLenPkgHeader || e_pkgLen >(_PKG_MAX_LENGTH - 1000))
    {
        // 非法包，重置状态
        pConn->curStat = _PKG_HD_INIT;
        pConn->precvbuf = pConn->dataHeadInfo;
        pConn->irecvlen = m_iLenPkgHeader;
        return;
    }

    // 分配消息缓冲区（消息头 + 包头 + 包体）
    auto pTmpBuffer = memory.AllocBuffer(m_iLenMsgHeader + e_pkgLen, false);
    char* pRawBuf = pTmpBuffer.get(); // 临时裸指针

    // ✅ 用普通结构体指针解析，不要用 unique_ptr 类型
    STRUC_MSG_HEADER* pMsgHeader = reinterpret_cast<STRUC_MSG_HEADER*>(pRawBuf);
    pMsgHeader->pConn = pConn;
    pMsgHeader->iCurrsequence = pConn->iCurrsequence;

    // 填写包头
    char* pPkgBuf = pRawBuf + m_iLenMsgHeader;
    std::memcpy(pPkgBuf, pPkgHeader, m_iLenPkgHeader);

    // 转移内存所有权
    pConn->precvMemPointer = std::move(pTmpBuffer);

    if (e_pkgLen == m_iLenPkgHeader)
    {
        if (m_floodAkEnable == 1)
            isflood = TestFlood(pConn);
        ngx_wait_request_handler_proc_plast(pConn, isflood);
    }
    else
    {
        pConn->curStat = _PKG_BD_INIT;
        pConn->precvbuf = pPkgBuf + m_iLenPkgHeader;
        pConn->irecvlen = e_pkgLen - m_iLenPkgHeader;
    }
}



//收到一个完整消息后，入消息队列，并调用inMsgRecvQueueAndSignal触发线程池中线程来处理该消息
void CSocekt::ngx_wait_request_handler_proc_plast(ngx_connection_sp pConn,bool &isflood)
{
    if(isflood == false)
    {
        g_threadpool.inMsgRecvQueueAndSignal(std::move(pConn->precvMemPointer)); //入消息队列并触发线程处理消息
    }
    else
    {
        //对于有攻击倾向的恶人，先把他的包丢掉
        pConn->precvMemPointer.reset();  // 释放内存
    }

    pConn->precvMemPointer = nullptr;
    pConn->curStat         = _PKG_HD_INIT;     //收包状态机的状态恢复为原始态，为收下一个包做准备                    
    pConn->precvbuf        = pConn->dataHeadInfo;  //设置好收包的位置
    pConn->irecvlen        = m_iLenPkgHeader;  //设置好要接收数据的大小
    return;
}



ssize_t CSocekt::sendproc(ngx_connection_sp c,char *buff,ssize_t size)
{
    ssize_t   n;
    for ( ;; )
    {
        n = send(c->fd, buff, size, 0); 
        if(n > 0) //成功发送了一些数据
        {        
            return n; //返回本次发送的字节数
        }

        if(n == 0)
        {
            ngx_log_stderr(errno, "CSocekt::sendproc()中send()返回0！");
            return 0; //断开连接
        }

        if(errno == EAGAIN)  
        {
            return -1;  //表示发送缓冲区满了
        }

        if(errno == EINTR) 
        {
            //这个不算错误 
            //参考官方的写法，打印个日志，其他啥也没干，那就是等下次for循环重新send试一次了
            ngx_log_stderr(errno,"CSocekt::sendproc()中send()失败.");  
        }
        else
        {
            ngx_log_stderr(errno, "CSocekt::sendproc()中send()出现其他错误！");
            return -2;
        }
    }
}

void CSocekt::ngx_write_request_handler(ngx_connection_sp pConn)
{
    ngx_log_stderr(errno, "ngx_write_request_handler被调用！！");
    if (!pConn || pConn->fd == -1) {
        ngx_log_stderr(0, "无效的连接对象");
        return;
    }
    // 发送本轮数据
    ssize_t sendsize = sendproc(pConn, pConn->psendbuf, pConn->isendlen);

    if (sendsize > 0)
    {
        if (sendsize == static_cast<ssize_t>(pConn->isendlen))
        {
            // 本次把剩余数据一次性发完 —— 发送完成
            if (ngx_epoll_oper_event(
                pConn->fd,
                EPOLL_CTL_MOD,       // 修改
                EPOLLOUT,            // 去掉写事件
                1,                   // 1：去掉
                pConn
            ) == -1)
            {
                ngx_log_stderr(errno, "CSocekt::ngx_write_request_handler()中ngx_epoll_oper_event()失败。");
            }

            // 释放并清理发送缓冲
            pConn->psendMemPointer.reset(); // 自动释放内存并置空
            pConn->psendbuf = nullptr;
            pConn->isendlen = 0;

            // 该连接的“投递写事件计数”减一
            --pConn->iThrowsendCount;

            ngx_log_stderr(0, "CSocekt::ngx_write_request_handler()中数据发送完毕!");
            return;
        }
        else
        {
            // 只发出一部分，继续等 EPOLLOUT 驱动
            ngx_log_stderr(errno,
                "ngx_write_request_handler发送了%d数据，实际要发送%d数据！！",
                static_cast<int>(sendsize),
                static_cast<int>(pConn->isendlen));

            pConn->psendbuf += sendsize;
            pConn->isendlen -= static_cast<unsigned int>(sendsize);
            return;
        }
    }
    else if (sendsize == -1)
    {
        // 一般是 EAGAIN：写缓冲区满。仍保持 EPOLLOUT 等下一次可写，不动缓冲。
        ngx_log_stderr(errno, "CSocekt::ngx_write_request_handler()时sendproc()返回-1，继续等待可写事件。");
        return;
    }
    else // sendsize == 0
    {
        // 对端关闭或其他异常情况：这时当前发送缓冲没意义了，清理掉
        ngx_log_stderr(errno, "CSocekt::ngx_write_request_handler()中sendproc()返回0，对端可能关闭，释放发送缓冲。");

        pConn->psendMemPointer.reset();
        pConn->psendbuf = nullptr;
        pConn->isendlen = 0;

        // 防止计数泄漏
        if (pConn->iThrowsendCount > 0)
            --pConn->iThrowsendCount;

        // 这里如果需要，后续可考虑关闭连接：
        ngx_close_connection(pConn);

        return;
    }
}



//虚函数，被子类重写
void CSocekt::threadRecvProcFunc(char *pMsgBuf)
{   
    return;
}


