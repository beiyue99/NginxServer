
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
#include "ngx_c_lockmutex.h"  





//调用recvproc接受数据，收完调用ngx_wait_request_handler_proc_plast入消息队列并激发线程处理
void CSocekt::ngx_read_request_handler(lpngx_connection_t pConn)
{  
    bool isflood = false; 

    ssize_t reco = recvproc(pConn,pConn->precvbuf,pConn->irecvlen); 
    //返回-1是断开或者错误
    if(reco <= 0)  
    {
        return;
    }

    if(pConn->curStat == _PKG_HD_INIT) 
    {        
        if(reco == m_iLenPkgHeader)//正好收到完整包头，这里拆解包头
        {   
            ngx_wait_request_handler_proc_p1(pConn,isflood); 
        }
        else
		{
            pConn->curStat        = _PKG_HD_RECVING;                
            pConn->precvbuf       = pConn->precvbuf + reco;          
            pConn->irecvlen       = pConn->irecvlen - reco;            
        }
    } 
    else if(pConn->curStat == _PKG_HD_RECVING) 
    {
        if(reco==pConn->irecvlen )
        {
            ngx_wait_request_handler_proc_p1(pConn,isflood);
        }
        else
		{
			//包头还是没收完整，继续收包头
            pConn->precvbuf       = pConn->precvbuf + reco;              //注意收后续包的内存往后走
            pConn->irecvlen       = pConn->irecvlen - reco;              //要收的内容当然要减少，以确保只收到完整的包头先
        }
    }
    else if(pConn->curStat == _PKG_BD_INIT) 
    {
        if(reco == pConn->irecvlen)
        {
            if(m_floodAkEnable == 1) 
            {
                isflood = TestFlood(pConn);  //如果频繁，FloodAttackCount++，达到一定次数就踢出
            }
            ngx_wait_request_handler_proc_plast(pConn,isflood);
        }
        else
		{
			pConn->curStat = _PKG_BD_RECVING;					
			pConn->precvbuf = pConn->precvbuf + reco;
			pConn->irecvlen = pConn->irecvlen - reco;
		}
    }
    else if(pConn->curStat == _PKG_BD_RECVING) 
    {
        if(pConn->irecvlen == reco)
        {
            //包体收完整了
            if(m_floodAkEnable == 1) 
            {
                //Flood攻击检测是否开启
                isflood = TestFlood(pConn);
            }
            ngx_wait_request_handler_proc_plast(pConn,isflood);
        }
        else
        {
            //包体没收完整，继续收
            pConn->precvbuf = pConn->precvbuf + reco;
			pConn->irecvlen = pConn->irecvlen - reco;
        }
    } 

    if(isflood == true)
    {
        ngx_log_stderr(errno,"发现客户端flood，踢掉该客户端!");
        zdClosesocketProc(pConn);
    }

    return;
}

ssize_t CSocekt::recvproc(lpngx_connection_t pConn,char *buff,ssize_t buflen)  //ssize_t是有符号整型，在32位机器上等同与int，在64位机器上等同与long int，size_t就是无符号型的ssize_t
{
    ssize_t n;
    n = recv(pConn->fd, buff, buflen, 0);
    if(n == 0)
    {
        ngx_log_stderr(0,"连接被客户端正常关闭[4路挥手关闭]！");
        zdClosesocketProc(pConn);        
        return -1;
    }
    if(n < 0) //这被认为有错误发生
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中errno == EAGAIN || errno == EWOULDBLOCK成立，出乎意料！");
            return -1;
        }
        if(errno == EINTR)
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中errno == EINTR成立，出乎意料！");
            return -1; 
        }

        if(errno == ECONNRESET)
        {
            ngx_log_stderr(errno, "CSocekt::recvproc()中errno ==ECONNRESET！");
            //do nothing
        }
        else
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中发生错误，我打印出来看看是啥错误！");  
        } 
        
        ngx_log_stderr(0,"连接被客户端非正常关闭！");

        zdClosesocketProc(pConn);
        return -1;
    }

    return n; //返回收到的字节数
}


//拆解包头，new出收pTmpBuffer指针，指向消息头+包头+包体大小的内存，拷贝包头进来，然后收包体
void CSocekt::ngx_wait_request_handler_proc_p1(lpngx_connection_t pConn,bool &isflood)
{    
    CMemory &memory = CMemory::GetInstance();		

    LPCOMM_PKG_HEADER pPkgHeader;
    pPkgHeader = (LPCOMM_PKG_HEADER)pConn->dataHeadInfo; //之前初始化了头指针指向dateHeadInfo

    unsigned short e_pkgLen; 
    e_pkgLen = ntohs(pPkgHeader->pkgLen);  
    //恶意包或者错误包的判断
    if(e_pkgLen < m_iLenPkgHeader) 
    {
        pConn->curStat = _PKG_HD_INIT;      
        pConn->precvbuf = pConn->dataHeadInfo;
        pConn->irecvlen = m_iLenPkgHeader;
    }
    else if(e_pkgLen > (_PKG_MAX_LENGTH-1000)) 
    {
        pConn->curStat = _PKG_HD_INIT;
        pConn->precvbuf = pConn->dataHeadInfo;
        pConn->irecvlen = m_iLenPkgHeader;
    }
    else
    {
        //合法的包头，继续处理
        char *pTmpBuffer  = (char *)memory.AllocMemory(m_iLenMsgHeader + e_pkgLen,false);
        pConn->precvMemPointer = pTmpBuffer;  //内存开始指针

        //a)先填写消息头内容
        LPSTRUC_MSG_HEADER ptmpMsgHeader = (LPSTRUC_MSG_HEADER)pTmpBuffer;
        ptmpMsgHeader->pConn = pConn;
        ptmpMsgHeader->iCurrsequence = pConn->iCurrsequence; 
        //b)再填写包头内容
        pTmpBuffer += m_iLenMsgHeader;                 //往后跳，跳过消息头，指向包头
        memcpy(pTmpBuffer,pPkgHeader,m_iLenPkgHeader); //直接把收到的包头拷贝进来
        if(e_pkgLen == m_iLenPkgHeader)
        {
            //该报文只有包头无包体【我们允许一个包只有包头，没有包体】
            if(m_floodAkEnable == 1) 
            {
                //Flood攻击检测是否开启
                isflood = TestFlood(pConn);
            }
            ngx_wait_request_handler_proc_plast(pConn,isflood);
        } 
        else
        {
            //开始收包体，注意我的写法
            pConn->curStat = _PKG_BD_INIT;                   //当前状态发生改变，包头刚好收完，准备接收包体	    
            pConn->precvbuf = pTmpBuffer + m_iLenPkgHeader;  
            pConn->irecvlen = e_pkgLen - m_iLenPkgHeader;    
        }                       
    } 

    return;
}



//收到一个完整消息后，入消息队列，并调用inMsgRecvQueueAndSignal触发线程池中线程来处理该消息
void CSocekt::ngx_wait_request_handler_proc_plast(lpngx_connection_t pConn,bool &isflood)
{
    if(isflood == false)
    {
        g_threadpool.inMsgRecvQueueAndSignal(pConn->precvMemPointer); //入消息队列并触发线程处理消息
    }
    else
    {
        //对于有攻击倾向的恶人，先把他的包丢掉
        CMemory &memory = CMemory::GetInstance();
        memory.FreeMemory(pConn->precvMemPointer); //直接释放掉内存，根本不往消息队列入
    }

    pConn->precvMemPointer = NULL;
    pConn->curStat         = _PKG_HD_INIT;     //收包状态机的状态恢复为原始态，为收下一个包做准备                    
    pConn->precvbuf        = pConn->dataHeadInfo;  //设置好收包的位置
    pConn->irecvlen        = m_iLenPkgHeader;  //设置好要接收数据的大小
    return;
}



ssize_t CSocekt::sendproc(lpngx_connection_t c,char *buff,ssize_t size)
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

void CSocekt::ngx_write_request_handler(lpngx_connection_t pConn)
{      
    ngx_log_stderr(errno, "ngx_write_request_handler被调用！！");
    CMemory &memory = CMemory::GetInstance();
    
    ssize_t sendsize = sendproc(pConn,pConn->psendbuf,pConn->isendlen);

    if(sendsize > 0 && sendsize != pConn->isendlen)
    {        
        
        ngx_log_stderr(errno, "ngx_write_request_handler发送了%d数据，实际要发送%d数据！！",sendsize,pConn->isendlen);
        //没有全部发送完毕，数据只发出去了一部分，那么发送到了哪里，剩余多少，继续记录，方便下次sendproc()时使用
        pConn->psendbuf = pConn->psendbuf + sendsize;
		pConn->isendlen = pConn->isendlen - sendsize;	
        return;
    }
    else if(sendsize == -1)
    {
        //这不太可能，可以发送数据时通知我发送数据，我发送时你却通知我发送缓冲区满？
        ngx_log_stderr(errno,"CSocekt::ngx_write_request_handler()时if(sendsize == -1)成立，这很怪异。");
        return;
    }

    if(sendsize > 0 && sendsize == pConn->isendlen) //成功发送完毕，做个通知是可以的；
    {
        if(ngx_epoll_oper_event(
                pConn->fd,          //socket句柄
                EPOLL_CTL_MOD,      //事件类型，这里是修改【因为我们准备减去写通知】
                EPOLLOUT,           //标志，这里代表要减去的标志,EPOLLOUT：可写【可写的时候通知我】
                1,                  //对于事件类型为增加的，EPOLL_CTL_MOD需要这个参数, 0：增加   1：去掉 2：完全覆盖
                pConn               //连接池中的连接
                ) == -1)
        {
            ngx_log_stderr(errno,"CSocekt::ngx_write_request_handler()中ngx_epoll_oper_event()失败。");
        }    

        ngx_log_stderr(0,"CSocekt::ngx_write_request_handler()中数据发送完毕!");
        
    }

    if(sem_post(&m_semEventSendQueue)==-1)       
        ngx_log_stderr(0,"CSocekt::ngx_write_request_handler()中sem_post(&m_semEventSendQueue)失败.");

    memory.FreeMemory(pConn->psendMemPointer);  //释放内存
    pConn->psendMemPointer = NULL;        
    --pConn->iThrowsendCount; 
    return;
}


//虚函数，被子类重写
void CSocekt::threadRecvProcFunc(char *pMsgBuf)
{   
    return;
}


