
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
#include <pthread.h>   //多线程

#include "ngx_c_conf.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_socket.h"
#include "ngx_c_memory.h"
#include "ngx_c_lockmutex.h"  //自动释放互斥量的一个类


//接收数据包存入连接，判断接受数据包的大小，检验完整性
void CSocekt::ngx_read_request_handler(lpngx_connection_t pConn)
{  
    // 因此我们必须保证 c->precvbuf指向正确的收包位置，保证c->irecvlen指向正确的收包宽度
    ssize_t reco = recvproc(pConn,pConn->precvbuf,pConn->irecvlen); 
    if(reco <= 0)  
    {
        return;    
    }

    //走到这里，说明成功收到了一些字节（>0），就要开始判断收到了多少数据了     
    if(pConn->curStat == _PKG_HD_INIT) //连接建立起来时就是这个状态
    {        
        if(reco == m_iLenPkgHeader)//正好收到完整包头，这里拆解包头
        {   
            ngx_wait_request_handler_proc_p1(pConn); //调用专门针对包头处理完整的函数去处理
        }
        else
		{
			//收到的包头不完整--我们不能预料每个包的长度，也不能预料各种拆包/粘包情况，所以收到不完整包头【也算是缺包】是很可能的；
            pConn->curStat        = _PKG_HD_RECVING;                 //接收包头中，包头不完整，继续接收包头中	
            pConn->precvbuf       = pConn->precvbuf + reco;              //注意收后续包的内存往后走
            pConn->irecvlen       = pConn->irecvlen - reco;              //要收的内容当然要减少，以确保只收到完整的包头先
        } 
    } 
    else if(pConn->curStat == _PKG_HD_RECVING) 
    {
        if(pConn->irecvlen == reco) //要求收到的宽度和我实际收到的宽度相等
        {
            //包头收完整了
            ngx_wait_request_handler_proc_p1(pConn); //那就调用专门针对包头处理完整的函数去处理把。
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
        //包头刚好收完，准备接收包体
        if(reco == pConn->irecvlen)
        {
            //收到的宽度等于要收的宽度，包体也收完整了
            ngx_wait_request_handler_proc_plast(pConn);
        }
        else
		{
			//收到的宽度小于要收的宽度
			pConn->curStat = _PKG_BD_RECVING;					
			pConn->precvbuf = pConn->precvbuf + reco;
			pConn->irecvlen = pConn->irecvlen - reco;
		}
    }
    else if(pConn->curStat == _PKG_BD_RECVING) 
    {
        //接收包体中，包体不完整，继续接收中
        if(pConn->irecvlen == reco)
        {
            //包体收完整了
            ngx_wait_request_handler_proc_plast(pConn);
        }
        else
        {
            //包体没收完整，继续收
            pConn->precvbuf = pConn->precvbuf + reco;
			pConn->irecvlen = pConn->irecvlen - reco;
        }
    }  
    return;
}


//收数据函数，被ngx_read_request_handler调用
ssize_t CSocekt::recvproc(lpngx_connection_t c,char *buff,ssize_t buflen) 
//ssize_t是有符号整型，在32位机器上等同与int，在64位机器上等同与long int，size_t就是无符号型的ssize_t
{
    ssize_t n;
    
    n = recv(c->fd, buff, buflen, 0); 
    if(n == 0)
    {
        //客户端关闭【应该是正常完成了4次挥手】，我这边就直接回收连接，关闭socket即可 
        ngx_log_stderr(0,"客户端正常关闭连接！");
        if(close(c->fd) == -1)
        {
            ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocekt::recvproc()中close(%d)失败!",c->fd);  
        }
        inRecyConnectQueue(c);
        return -1;
    }
    if(n < 0) //这被认为有错误发生
    {

        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中errno == EAGAIN || errno == EWOULDBLOCK成立！");
            return -1; //不当做错误处理，只是简单返回
        }
        if(errno == EINTR)  
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中errno == EINTR成立！");
            return -1; //不当做错误处理，只是简单返回
        }
             
        if(errno == ECONNRESET)  //#define ECONNRESET 104 /* Connection reset by peer */
        {
            ngx_log_stderr(errno, "CSocekt::recvproc()中errno == ECONNRESET！");
        }
        else
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中发生未知错误！"); 
        } 
        
        ngx_log_stderr(0,"连接被客户端非正常关闭！");
        if(close(c->fd) == -1)
        {
            ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocekt::recvproc()中close_2(%d)失败!",c->fd);  
        }
        inRecyConnectQueue(c);
        return -1;
    }
    printf("收到了%ld个字节的数据！\n", n);
    return n; //返回收到的字节数
}




//包头收完整后的处理，我们称为包处理阶段1【p1】，被ngx_read_request_handler调用
void CSocekt::ngx_wait_request_handler_proc_p1(lpngx_connection_t pConn)
{
    CMemory *p_memory = CMemory::GetInstance();		

    LPCOMM_PKG_HEADER pPkgHeader = (LPCOMM_PKG_HEADER)pConn->dataHeadInfo; 
    unsigned short e_pkgLen; 
    e_pkgLen = ntohs(pPkgHeader->pkgLen);  
    //注意这里网络序转本机序，所有传输到网络上的2字节数据，都要用htons()转成网络序，所有从网络上收到的2字节数据，都要用ntohs()转成本机序
   
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
        char *pTmpBuffer  = (char *)p_memory->AllocMemory(m_iLenMsgHeader + e_pkgLen,false);
        //分配内存【长度是 消息头长度  + 包头长度 + 包体长度】
        pConn->precvMemPointer = pTmpBuffer;  //内存开始指针

        //a)先填写消息头内容
        LPSTRUC_MSG_HEADER ptmpMsgHeader = (LPSTRUC_MSG_HEADER)pTmpBuffer;
        ptmpMsgHeader->pConn = pConn;
        ptmpMsgHeader->iCurrsequence = pConn->iCurrsequence; //收到包时的连接池中连接序号记录到消息头里来，以备将来用；
        //b)再填写包头内容
        pTmpBuffer += m_iLenMsgHeader;                 //往后跳，跳过消息头，指向包头
        memcpy(pTmpBuffer,pPkgHeader,m_iLenPkgHeader); //直接把收到的包头拷贝进来
        if(e_pkgLen == m_iLenPkgHeader)
        {
            //该报文只有包头无包体【我们允许一个包只有包头，没有包体】
            ngx_wait_request_handler_proc_plast(pConn);
        } 
        else
        {
            //开始收包体，注意我的写法
            pConn->curStat = _PKG_BD_INIT;                   //当前状态发生改变，包头刚好收完，准备接收包体	    
            pConn->precvbuf = pTmpBuffer + m_iLenPkgHeader;  //pTmpBuffer指向包头，这里 + m_iLenPkgHeader后指向包体 
            pConn->irecvlen = e_pkgLen - m_iLenPkgHeader;    //e_pkgLen是整个包【包头+包体】大小，-m_iLenPkgHeader【包头】  = 包体
        }                       
    }  

    return;
}


//该函数将完整数据包存入消息队列并且唤醒线程处理，被ngx_read_request_handler调用
void CSocekt::ngx_wait_request_handler_proc_plast(lpngx_connection_t pConn)
{


    g_threadpool.inMsgRecvQueueAndSignal(pConn->precvMemPointer); //入消息队列并触发（唤醒）线程处理消息
    
    pConn->precvMemPointer = NULL;
    pConn->curStat         = _PKG_HD_INIT;     //收包状态机的状态恢复为原始态，为收下一个包做准备                    
    pConn->precvbuf        = pConn->dataHeadInfo;  //设置好收包的位置
    pConn->irecvlen        = m_iLenPkgHeader;  //设置好要接收数据的大小
    return;
}


//发送数据，被ngx_write_request_handler调用，发送缓冲区可写的时候用来发送剩余的数据
ssize_t CSocekt::sendproc(lpngx_connection_t c, char* buff, ssize_t size)
{
	ssize_t   n;

	n = send(c->fd, buff, size, 0); //send()系统函数， 最后一个参数flag，一般为0； 
	if (n > 0) //成功发送了一些数据
	{
		return n; //返回本次发送的字节数
	}

	if (n == 0)
	{
		//网上找资料：send=0表示超时，对方主动关闭了连接过程，连接断开，集中到recv那里处理
		return 0;
	}

	if (errno == EAGAIN || errno == EWOULDBLOCK)  //这东西应该等于EWOULDBLOCK
	{
		//内核缓冲区满，这个不算错误
		return -1;  //表示发送缓冲区满了
	}

	if (errno == EINTR)
	{
		ngx_log_stderr(errno, "CSocekt::sendproc()中send()失败.");
		return -2;
	}
	else
	{
		// 等待recv()来统一处理断开
		return -2;
	}
}




//因为写缓冲区满而没发送完的数据，通过EPLOUT事件通知可写的时候，调用此函数继续发送剩余的数据
void CSocekt::ngx_write_request_handler(lpngx_connection_t pConn)
{      
    CMemory *p_memory = CMemory::GetInstance();
    
    ssize_t sendsize = sendproc(pConn,pConn->psendbuf,pConn->isendlen);

    if(sendsize > 0 && sendsize != pConn->isendlen)
    {        
        pConn->psendbuf = pConn->psendbuf + sendsize;
		pConn->isendlen = pConn->isendlen - sendsize;	
        return;
    }
    else if(sendsize == -1)
    {
        ngx_log_stderr(errno,"CSocekt::ngx_write_request_handler()使出错！"); //打印个日志，别的先不干啥
        return;
    }

    if(sendsize > 0 && sendsize == pConn->isendlen) //成功发送完毕，做个通知是可以的；
    {
        //如果是成功的发送完毕数据，则把写事件通知从epoll中干掉
        if(ngx_epoll_oper_event(
                pConn->fd,          //socket句柄
                EPOLL_CTL_MOD,      //事件类型，这里是修改【因为我们准备减去写通知】
                EPOLLOUT,           //标志，这里代表要减去的标志,EPOLLOUT
                1,                  //对于事件类型为增加的，EPOLL_CTL_MOD需要这个参数, 0：增加   1：去掉 2：完全覆盖
                pConn               //连接池中的连接
                ) == -1)
        {
            ngx_log_stderr(errno,"CSocekt::ngx_write_request_handler()中ngx_epoll_oper_event()失败。");
        }    
        ngx_log_stderr(0,"CSocekt::ngx_write_request_handler()中数据发送完毕！"); 
        
    }

    //数据包发送真正完毕，让发送线程往下走判断能否发送新数据
    if(sem_post(&m_semEventSendQueue)==-1)       
        ngx_log_stderr(0,"CSocekt::ngx_write_request_handler()中sem_post(&m_semEventSendQueue)失败.");

    p_memory->FreeMemory(pConn->psendMemPointer);  //释放内存
    pConn->psendMemPointer = NULL;        
    --pConn->iThrowsendCount;  //建议放在最后执行
    return;
}






