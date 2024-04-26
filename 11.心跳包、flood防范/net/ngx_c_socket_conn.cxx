
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
#include "ngx_c_memory.h"
#include "ngx_c_lockmutex.h"

ngx_connection_s::ngx_connection_s()//构造函数
{		
    iCurrsequence = 0;    
    pthread_mutex_init(&logicPorcMutex, NULL); //互斥量初始化
}
ngx_connection_s::~ngx_connection_s()//析构函数
{
    pthread_mutex_destroy(&logicPorcMutex);    //互斥量释放
}

//初始化连接
void ngx_connection_s::GetOneToUse()
{
    ++iCurrsequence;

    fd  = -1;                                         //开始先给-1
    curStat = _PKG_HD_INIT;                           //收包状态处于 初始状态，准备接收数据包头【状态机】
    precvbuf = dataHeadInfo;                          //收包我要先收到这里来，因为我要先收包头，所以收数据的buff直接就是dataHeadInfo
    irecvlen = sizeof(COMM_PKG_HEADER);               //这里指定收数据的长度，这里先要求收包头这么长字节的数据
    
    precvMemPointer = NULL;                           //既然没new内存，那自然指向的内存地址先给NULL
    iThrowsendCount = 0;                              //原子的
    psendMemPointer = NULL;                           //发送数据头指针记录
    events          = 0;                              //epoll事件先给0 
    lastPingTime    = time(NULL);                     //上次ping的时间

    FloodkickLastTime = 0;                            //Flood攻击上次收到包的时间
	FloodAttackCount  = 0;	                          //Flood攻击在该时间内收到包的次数统计
}

//回收回来一个连接  iCurrsequence++，释放接受和发送缓冲区的内存
void ngx_connection_s::PutOneToFree()
{
    ++iCurrsequence;   
    if(precvMemPointer != NULL)//我们曾经给这个连接分配过接收数据的内存，则要释放内存
    {        
        CMemory::GetInstance()->FreeMemory(precvMemPointer);
        precvMemPointer = NULL;        
    }
    if(psendMemPointer != NULL) //如果发送数据的缓冲区里有内容，则要释放内存
    {
        CMemory::GetInstance()->FreeMemory(psendMemPointer);
        psendMemPointer = NULL;
    }
    iThrowsendCount = 0;                          
}



//---------------------------------------------------------------
//创建连接，放入连接链表（连接池）
void CSocekt::initconnection()
{
    lpngx_connection_t p_Conn;
    CMemory *p_memory = CMemory::GetInstance();   

    int ilenconnpool = sizeof(ngx_connection_t);    
    for(int i = 0; i < m_worker_connections; ++i)
    {
        p_Conn = (lpngx_connection_t)p_memory->AllocMemory(ilenconnpool,true); 
        p_Conn = new(p_Conn) ngx_connection_t();  
        p_Conn->GetOneToUse(); //初始化连接
        m_connectionList.push_back(p_Conn);     //所有链接【不管是否空闲】都放在这个list
        m_freeconnectionList.push_back(p_Conn); //空闲连接会放在这个list
    } 
    m_free_connection_n = m_total_connection_n = m_connectionList.size(); //开始这两个列表一样大
    return;
}

//最终回收连接池，释放内存
void CSocekt::clearconnection()
{
    lpngx_connection_t p_Conn;
	CMemory *p_memory = CMemory::GetInstance();
	
	while(!m_connectionList.empty())
	{
		p_Conn = m_connectionList.front();
		m_connectionList.pop_front(); 
        p_Conn->~ngx_connection_t();     //手工调用析构函数
		p_memory->FreeMemory(p_Conn);
	}
}



//返回一个连接，与传参的套间字绑定
lpngx_connection_t CSocekt::ngx_get_connection(int isock)
{
    CLock lock(&m_connectionMutex);  

    if(!m_freeconnectionList.empty())
    {
        lpngx_connection_t p_Conn = m_freeconnectionList.front(); //返回第一个元素但不检查元素存在与否
        m_freeconnectionList.pop_front();                         //移除第一个元素但不返回	
        p_Conn->GetOneToUse();
        --m_free_connection_n; 
        p_Conn->fd = isock;
        return p_Conn;
    }

    //走到这里，表示没空闲的连接了，那就考虑重新创建一个连接
    CMemory *p_memory = CMemory::GetInstance();
    lpngx_connection_t p_Conn = (lpngx_connection_t)p_memory->AllocMemory(sizeof(ngx_connection_t),true);
    p_Conn = new(p_Conn) ngx_connection_t();
    p_Conn->GetOneToUse();
    m_connectionList.push_back(p_Conn); //入到总表中来，但不能入到空闲表中来，因为凡是调这个函数的，肯定是要用这个连接的
    ++m_total_connection_n;             
    p_Conn->fd = isock;
    return p_Conn;
}



//将一个连接回收，放入空闲连接链表m_freeconnectionList，被ServerRecyConnectionThread线程函数调用
void CSocekt::ngx_free_connection(lpngx_connection_t pConn) 
{
    CLock lock(&m_connectionMutex);  
    pConn->PutOneToFree();  //释放连接所持有的空间资源
    m_freeconnectionList.push_back(pConn);
    ++m_free_connection_n;
    return;
}


//把连接放入m_recyconnectionList回收队列，被zdClosesocketProc函数调用
void CSocekt::inRecyConnectQueue(lpngx_connection_t pConn)
{
    std::list<lpngx_connection_t>::iterator pos;
    bool iffind = false;
    CLock lock(&m_recyconnqueueMutex); 
    for(pos = m_recyconnectionList.begin(); pos != m_recyconnectionList.end(); ++pos)
	{
		if((*pos) == pConn)		
		{	
			iffind = true;
			break;			
		}
	}
    if(iffind == true) //找到了，不必再入了
	{
        return;
    }
    pConn->inRecyTime = time(NULL);        //记录回收时间
    ++pConn->iCurrsequence;
    m_recyconnectionList.push_back(pConn); //等待ServerRecyConnectionThread线程自会处理 
    ++m_totol_recyconnection_n;            //待释放连接队列大小+1
    --m_onlineUserCount;                   //连入用户数量-1
    return;
}



//处理连接回收的线程，如果到时间了，调用ngx_free_connection回收连接
void* CSocekt::ServerRecyConnectionThread(void* threadData)
{
    ThreadItem *pThread = static_cast<ThreadItem*>(threadData);
    CSocekt *pSocketObj = pThread->_pThis;
    
    time_t currtime;
    int err;
    std::list<lpngx_connection_t>::iterator pos,posend;
    lpngx_connection_t p_Conn;
    
    while(1)
    {
        usleep(200 * 1000);  //单位是微妙,又因为1毫秒=1000微妙，所以 200 *1000 = 200毫秒
        if(pSocketObj->m_totol_recyconnection_n > 0)
        {
            currtime = time(NULL);
            err = pthread_mutex_lock(&pSocketObj->m_recyconnqueueMutex);  
            if(err != 0) ngx_log_stderr(err,"CSocekt::ServerRecyConnectionThread()中pthread_mutex_lock()失败，返回的错误码为%d!",err);
lblRRTD:
            pos    = pSocketObj->m_recyconnectionList.begin();
			posend = pSocketObj->m_recyconnectionList.end();
            for(; pos != posend; ++pos)
            {
                p_Conn = (*pos);
                if(
                    ( (p_Conn->inRecyTime + pSocketObj->m_RecyConnectionWaitTime) > currtime)  && (g_stopEvent == 0)
                    )
                {
                    continue; //没到释放的时间
                }    
                if(p_Conn->iThrowsendCount > 0)
                {
                    ngx_log_stderr(0,"CSocekt::ServerRecyConnectionThread()中到释放时间却发现p_Conn.iThrowsendCount!=0，这个不该发生");
                }

                --pSocketObj->m_totol_recyconnection_n;        //待释放连接队列大小-1
                pSocketObj->m_recyconnectionList.erase(pos); 
                pSocketObj->ngx_free_connection(p_Conn);
                //将一个连接回收，放入空闲连接链表m_freeconnectionList,然后调用PutOneToFree释放内存，回收连接

                goto lblRRTD; 
            }
            err = pthread_mutex_unlock(&pSocketObj->m_recyconnqueueMutex); 
            if(err != 0)  ngx_log_stderr(err,"CSocekt::ServerRecyConnectionThread()pthread_mutex_unlock()失败，返回的错误码为%d!",err);
        } 
        if(g_stopEvent == 1) 
        {
            if(pSocketObj->m_totol_recyconnection_n > 0)
            {
                err = pthread_mutex_lock(&pSocketObj->m_recyconnqueueMutex);  
                if(err != 0) ngx_log_stderr(err,"CSocekt::ServerRecyConnectionThread()中pthread_mutex_lock2()失败，返回的错误码为%d!",err);

        lblRRTD2:
                pos    = pSocketObj->m_recyconnectionList.begin();
			    posend = pSocketObj->m_recyconnectionList.end();
                for(; pos != posend; ++pos)
                {
                    p_Conn = (*pos);
                    --pSocketObj->m_totol_recyconnection_n;        //待释放连接队列大小-1
                    pSocketObj->m_recyconnectionList.erase(pos);  
                    pSocketObj->ngx_free_connection(p_Conn);	  
                    goto lblRRTD2; 
                } 
                err = pthread_mutex_unlock(&pSocketObj->m_recyconnqueueMutex); 
                if(err != 0)  ngx_log_stderr(err,"CSocekt::ServerRecyConnectionThread()pthread_mutex_unlock2()失败，返回的错误码为%d!",err);
            } 
            break; 
        }
    }  
    return (void*)0;
}

void CSocekt::ngx_close_connection(lpngx_connection_t pConn)
{    
    ngx_free_connection(pConn); 
    //将一个连接回收，放入空闲连接链表m_freeconnectionList
    if(pConn->fd != -1)
    {
        close(pConn->fd);
        pConn->fd = -1;
    }    
    return;
}
