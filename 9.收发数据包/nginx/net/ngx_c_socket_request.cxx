
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
#include "ngx_c_memory.h"

void CSocekt::ngx_wait_request_handler(lpngx_connection_t c)
{  
    // 因此我们必须保证 c->precvbuf指向正确的收包位置，保证c->irecvlen指向正确的收包宽度
    ssize_t reco = recvproc(c,c->precvbuf,c->irecvlen); //返回读到的字节数
    if(reco <= 0)  
    {
        return;
    }

    if(c->curStat == _PKG_HD_INIT)
        //连接建立起来时肯定是这个状态，因为在ngx_get_connection()中已经把curStat成员赋值成_PKG_HD_INIT了
    {        
        if(reco == m_iLenPkgHeader)//正好收到完整包头，这里拆解包头
        {   
            ngx_wait_request_handler_proc_p1(c); 
        }
        else
		{
			//收到的包头不完整--我们不能预料每个包的长度，也不能预料各种拆包/粘包情况，
            // 所以收到不完整包头【也算是缺包】是很可能的；
            c->curStat        = _PKG_HD_RECVING;                 //接收包头中，包头不完整，继续接收包头中	
            c->precvbuf       = c->precvbuf + reco;              //注意收后续包的内存往后走
            c->irecvlen       = c->irecvlen - reco;              //要收的内容当然要减少，以确保只收到完整的包头先
        } 
    } 
    else if(c->curStat == _PKG_HD_RECVING) //接收包头中，包头不完整，继续接收中，这个条件才会成立
    {
        if(c->irecvlen == reco) //要求收到的宽度和我实际收到的宽度相等
        {
            //包头收完整了
            ngx_wait_request_handler_proc_p1(c); //那就调用专门针对包头处理完整的函数去处理把。
        }
        else
		{
			//包头还是没收完整，继续收包头
            c->precvbuf       = c->precvbuf + reco;              //注意收后续包的内存往后走
            c->irecvlen       = c->irecvlen - reco;              //要收的内容当然要减少，以确保只收到完整的包头先
        }
    }
    else if(c->curStat == _PKG_BD_INIT) 
    {
        //包头刚好收完，准备接收包体
        if(reco == c->irecvlen)
        {
            //收到的宽度等于要收的宽度，包体也收完整了
            ngx_wait_request_handler_proc_plast(c);
        }
        else
		{
			//收到的宽度小于要收的宽度
			c->curStat = _PKG_BD_RECVING;					
			c->precvbuf = c->precvbuf + reco;
			c->irecvlen = c->irecvlen - reco;
		}
    }
    else if(c->curStat == _PKG_BD_RECVING) 
    {
        //接收包体中，包体不完整，继续接收中
        if(c->irecvlen == reco)
        {
            //包体收完整了
            ngx_wait_request_handler_proc_plast(c);
        }
        else
        {
            //包体没收完整，继续收
            c->precvbuf = c->precvbuf + reco;
			c->irecvlen = c->irecvlen - reco;
        }
    }  
    return;
}

ssize_t CSocekt::recvproc(lpngx_connection_t c,char *buff,ssize_t buflen) 
//ssize_t是有符号整型，在32位机器上等同与int，在64位机器上等同与long int，size_t就是无符号型的ssize_t
{
    ssize_t n;
    
    n = recv(c->fd, buff, buflen, 0); //recv()系统函数， 最后一个参数flag，一般为0；     
    if(n == 0)
    {
        ngx_close_connection(c);
        return -1;
    }
    //客户端没断，走这里 
    if(n < 0) //这被认为有错误发生
    {

        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中errno == EAGAIN || errno == EWOULDBLOCK成立，出乎我意料！");
            return -1; //不当做错误处理，只是简单返回
        }
        if(errno == EINTR)  
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中errno == EINTR成立，出乎我意料！");
            return -1; //不当做错误处理，只是简单返回
        }

        if(errno == ECONNRESET)  //#define ECONNRESET 104 /* Connection reset by peer */
        {
            //算常规错误吧【普通信息型】，日志都不用打印，没啥意思，太普通的错误
            //do nothing
            ngx_log_stderr(errno, "CSocekt::recvproc()中errno == ECONNRESET！");
        }
        else
        {
            ngx_log_stderr(errno,"CSocekt::recvproc()中发生错误，我打印出来看看是啥错误！");  //正式运营时可以考虑这些日志打印去掉
        } 
        
        ngx_log_stderr(0,"连接被客户端非正常关闭！");

        ngx_close_connection(c);
        return -1;
    }

    return n; //返回收到的字节数
}


//处理包头，接收包体，p1阶段
void CSocekt::ngx_wait_request_handler_proc_p1(lpngx_connection_t c)
{
    CMemory *p_memory = CMemory::GetInstance();		
    LPCOMM_PKG_HEADER pPkgHeader;
    pPkgHeader = (LPCOMM_PKG_HEADER)c->dataHeadInfo; //正好收到包头时，包头信息肯定是在dataHeadInfo里；
    unsigned short e_pkgLen; 
    e_pkgLen = ntohs(pPkgHeader->pkgLen);  
    //恶意包或者错误包的判断
    if(e_pkgLen < m_iLenPkgHeader) 
    {
        //报文总长度 < 包头长度，认定非法用户，废包
        c->curStat = _PKG_HD_INIT;      
        c->precvbuf = c->dataHeadInfo;
        c->irecvlen = m_iLenPkgHeader;
    }
    else if(e_pkgLen > (_PKG_MAX_LENGTH-1000))   //客户端发来包居然说包长度 > 29000?肯定是恶意包
    {
        c->curStat = _PKG_HD_INIT;
        c->precvbuf = c->dataHeadInfo;
        c->irecvlen = m_iLenPkgHeader;
    }
    else
    {
        //合法的包头，继续处理
        //我现在要分配内存开始收包体，因为包体长度并不是固定的，所以内存肯定要new出来；
        char *pTmpBuffer  = (char *)p_memory->AllocMemory(m_iLenMsgHeader + e_pkgLen,false);
        //分配内存【长度是 消息头长度  + 包头长度 + 包体长度】，最后参数先给false，表示内存不需要memset;
        c->ifnewrecvMem   = true;        //标记我们new了内存，将来在ngx_free_connection()要回收的
        c->pnewMemPointer = pTmpBuffer;  //内存开始指针

        //a)先填写消息头内容
        LPSTRUC_MSG_HEADER ptmpMsgHeader = (LPSTRUC_MSG_HEADER)pTmpBuffer;
        ptmpMsgHeader->pConn = c;
        ptmpMsgHeader->iCurrsequence = c->iCurrsequence; //收到包时的连接池中连接序号记录到消息头里来，以备将来用；
        //b)再填写包头内容
        pTmpBuffer += m_iLenMsgHeader;                 //往后跳，跳过消息头，指向包头
        memcpy(pTmpBuffer,pPkgHeader,m_iLenPkgHeader); //直接把收到的包头拷贝进来
        if(e_pkgLen == m_iLenPkgHeader)
        {
            //只有包头没有包体，这相当于收完整了，则直接入消息队列待后续业务逻辑线程去处理吧
            ngx_wait_request_handler_proc_plast(c);
        } 
        else
        {
            c->curStat = _PKG_BD_INIT;                   //当前状态发生改变，包头刚好收完，准备接收包体	    
            c->precvbuf = pTmpBuffer + m_iLenPkgHeader;  //pTmpBuffer指向包头，这里 + m_iLenPkgHeader后指向包体 
            c->irecvlen = e_pkgLen - m_iLenPkgHeader;    //e_pkgLen是整个包【包头+包体】大小，-m_iLenPkgHeader【包头】  = 包体
        }                       
    }  
    return;
}

//收到一个完整包后的处理【plast表示最后阶段】，放到一个函数中，方便调用
void CSocekt::ngx_wait_request_handler_proc_plast(lpngx_connection_t c)
{
    //把这段内存放到消息队列中来；
    inMsgRecvQueue(c->pnewMemPointer);
    //......这里考虑触发业务逻辑，怎么触发业务逻辑，这个代码以后再考虑扩充。。。。。。
    


    c->ifnewrecvMem    = false;            
    //内存不再需要释放，因为你收完整了包，这个包被上边调用inMsgRecvQueue()移入消息队列
    c->pnewMemPointer  = NULL;
    c->curStat         = _PKG_HD_INIT;     //收包状态机的状态恢复为原始态，为收下一个包做准备                    
    c->precvbuf        = c->dataHeadInfo;  //设置好收包的位置
    c->irecvlen        = m_iLenPkgHeader;  //设置好要接收数据的大小
    return;
}



//当收到一个完整包之后，将完整包入消息队列，这个包在服务器端应该是 消息头+包头+包体 格式
void CSocekt::inMsgRecvQueue(char *buf) //buf这段内存 ： 消息头 + 包头 + 包体
{
    m_MsgRecvQueue.push_back(buf);	

    //....其他功能待扩充，这里要记住一点，这里的内存都是要释放的

    ngx_log_stderr(0,"非常好，收到了一个完整的数据包【包头+包体】！");  
}

