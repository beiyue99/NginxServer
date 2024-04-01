
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
#include "ngx_c_lockmutex.h"



//在队列插入键值对（20秒后时间，消息头），并把队头时间保存到m_timer_value_里
void CSocket::AddToTimerQueue(lpngx_connection_t pConn)
{
    CMemory *p_memory = CMemory::GetInstance();
    time_t futtime = time(NULL);
    futtime += m_iWaitTime;  //20秒之后的时间


    CLock lock(&m_timequeueMutex); //互斥，因为要操作m_timeQueuemap了
	//创建一个消息头，消息头记录了当前连接和iCurrsequence
    LPSTRUC_MSG_HEADER tmpMsgHeader = (LPSTRUC_MSG_HEADER)p_memory->AllocMemory(m_iLenMsgHeader,false);
    tmpMsgHeader->pConn = pConn;
    tmpMsgHeader->iCurrsequence = pConn->iCurrsequence;
    m_timerQueuemap.insert(std::make_pair(futtime,tmpMsgHeader)); //按键 自动排序 小->大
    m_cur_size_++;  //计时队列尺寸+1
    m_timer_value_ = GetEarliestTime(); //计时队列头部时间值保存到m_timer_value_（队列头部时间）里
    return;    
}

//从multimap中取得最早的时间，不删除
time_t CSocket::GetEarliestTime()
{
    std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos;	
	pos = m_timerQueuemap.begin();		
	return pos->first;	
}


//从m_timeQueuemap移除最早的时间，返回消息头
LPSTRUC_MSG_HEADER CSocket::RemoveFirstTimer()
{
	std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos;	
	LPSTRUC_MSG_HEADER p_tmp;
	if(m_cur_size_ <= 0)
	{
		return NULL;
	}
	pos = m_timerQueuemap.begin();
	p_tmp = pos->second;
	m_timerQueuemap.erase(pos);
	--m_cur_size_;
	return p_tmp;
}


//根据给的当前时间，从m_timeQueuemap找到比这个时间更老（更早）的节点【1个】返回去，这些节点都是时间超过了
//然后往map插入新的键值对(curtime+waittime) 并同样储存在m_timer_valua_里
LPSTRUC_MSG_HEADER CSocket::GetOverTimeTimer(time_t cur_time)
{	
	CMemory *p_memory = CMemory::GetInstance();
	LPSTRUC_MSG_HEADER ptmp;
	if (m_cur_size_ == 0 || m_timerQueuemap.empty())
		return NULL; //队列为空


	time_t earliesttime = GetEarliestTime(); //到multimap中去查询
	if (earliesttime <= cur_time)  
	{
		ptmp = RemoveFirstTimer();    //把这个超时的节点从 m_timerQueuemap 删掉，并把这个节点的第二项返回来；

		if(m_ifTimeOutKick != 1) 
		{
			//下次超时的时间我们也依然要判断    
			time_t newinqueutime = cur_time+(m_iWaitTime);
			LPSTRUC_MSG_HEADER tmpMsgHeader = (LPSTRUC_MSG_HEADER)p_memory->AllocMemory(sizeof(STRUC_MSG_HEADER),false);
			tmpMsgHeader->pConn = ptmp->pConn;
			tmpMsgHeader->iCurrsequence = ptmp->iCurrsequence;			
			m_timerQueuemap.insert(std::make_pair(newinqueutime,tmpMsgHeader)); 		
			m_cur_size_++;       
		}

		if(m_cur_size_ > 0) 
		{
			m_timer_value_ = GetEarliestTime(); //计时队列头部时间值保存到m_timer_value_里
		}
		return ptmp;
	}
	return NULL;
}


//把指定用户tcp连接从timer表中删除
void CSocket::DeleteFromTimerQueue(lpngx_connection_t pConn)
{
    std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos,posend;
	CMemory *p_memory = CMemory::GetInstance();
    CLock lock(&m_timequeueMutex);

lblMTQM:
	pos    = m_timerQueuemap.begin();
	posend = m_timerQueuemap.end();
	for(; pos != posend; ++pos)	
	{
		if(pos->second->pConn == pConn)
		{			
			p_memory->FreeMemory(pos->second);  //释放内存
			m_timerQueuemap.erase(pos);
			--m_cur_size_; //减去一个元素，必然要把尺寸减少1个;								
			goto lblMTQM;
		}		
	}
	if(m_cur_size_ > 0)
	{
		m_timer_value_ = GetEarliestTime();
	}
    return;    
}



//清理时间队列中所有内容
void CSocket::clearAllFromTimerQueue()
{	
	std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos,posend;

	CMemory *p_memory = CMemory::GetInstance();	
	pos    = m_timerQueuemap.begin();
	posend = m_timerQueuemap.end();    
	for(; pos != posend; ++pos)	
	{
		p_memory->FreeMemory(pos->second);		
		--m_cur_size_; 		
	}
	m_timerQueuemap.clear();
}




//时间队列监视和处理线程，调用procPingTimeOutChecking处理到期不发心跳包的用户踢出的线程
void* CSocket::ServerTimerQueueMonitorThread(void* threadData)
{
    ThreadItem *pThread = static_cast<ThreadItem*>(threadData);
    CSocket *pSocketObj = pThread->_pThis;
    time_t absolute_time,cur_time;
    int err;
    while(g_stopEvent == 0) //不退出
    {
		if(pSocketObj->m_cur_size_ > 0)//队列不为空，有内容
        {
            absolute_time = pSocketObj->m_timer_value_; //先不互斥，大致判断
            cur_time = time(NULL);
            if(absolute_time < cur_time)
            {
                std::list<LPSTRUC_MSG_HEADER> m_lsIdleList; //保存要处理的内容
                LPSTRUC_MSG_HEADER result;
				//严格判断，因为很可能有超时事件
                err = pthread_mutex_lock(&pSocketObj->m_timequeueMutex);  
                if(err != 0) ngx_log_stderr(err,"CSocket::ServerTimerQueueMonitorThread()中pthread_mutex_lock()失败，返回的错误码为%d!",err);
				//有问题，要及时报告
                while ((result = pSocketObj->GetOverTimeTimer(cur_time)) != NULL) //一次性的把所有超时节点都拿过来
				{
					//printf("把一个心跳包超时节点放入list！\n");
					//这里放入list的都是超时一个wait时间的，我们要超时三次+10个时间才会踢出连接
					m_lsIdleList.push_back(result); 
				}
                err = pthread_mutex_unlock(&pSocketObj->m_timequeueMutex); 
                if(err != 0)  ngx_log_stderr(err,"CSocket::ServerTimerQueueMonitorThread()pthread_mutex_unlock()失败，返回的错误码为%d!",err);
				//有问题，要及时报告                
                LPSTRUC_MSG_HEADER tmpmsg;
                while(!m_lsIdleList.empty())
                {
                    tmpmsg = m_lsIdleList.front();
					m_lsIdleList.pop_front(); 
                    pSocketObj->procPingTimeOutChecking(tmpmsg,cur_time); //这里需要检查心跳超时问题
                } 
            }
        }
        usleep(500 * 1000);
    } 
    return (void*)0;
}




//心跳包检测时间到，该去检测心跳包是否超时的事宜，本函数空实现
void CSocket::procPingTimeOutChecking(LPSTRUC_MSG_HEADER tmpmsg,time_t cur_time)
{

}


