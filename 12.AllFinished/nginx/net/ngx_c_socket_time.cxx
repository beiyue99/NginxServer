
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

//make_pair(futtime,tmpMsgHeader)   new消息头放入multimap，存连接和时间
void CSocekt::AddToTimerQueue(lpngx_connection_t pConn)
{
    CMemory &memory = CMemory::GetInstance();

    time_t futtime = time(NULL);
    futtime += m_iWaitTime;  

    CLock lock(&m_timequeueMutex); //互斥，因为要操作m_timeQueuemap了
    LPSTRUC_MSG_HEADER tmpMsgHeader = (LPSTRUC_MSG_HEADER)memory.AllocMemory(m_iLenMsgHeader,false);
    tmpMsgHeader->pConn = pConn;
    tmpMsgHeader->iCurrsequence = pConn->iCurrsequence;
    m_timerQueuemap.insert(std::make_pair(futtime,tmpMsgHeader)); //按键 自动排序 小->大
    m_cur_size_++;  //计时队列尺寸+1
    m_timer_value_ = GetEarliestTime(); //计时队列头部时间值保存到m_timer_value_里
    return;    
}



//从multimap中取得最早的时间返回去,储存在m_timer_value_
time_t CSocekt::GetEarliestTime()
{
    std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos;	
	pos = m_timerQueuemap.begin();		
	return pos->first;	
}



//从m_timeQueuemap移除最早的时间，并把最早这个时间所在的项的消息头指针返回
LPSTRUC_MSG_HEADER CSocekt::RemoveFirstTimer()
{
	std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos;	
	LPSTRUC_MSG_HEADER p_tmp;
	if(m_cur_size_ <= 0)
	{
		return NULL;
	}
	pos = m_timerQueuemap.begin(); //调用者负责互斥的，这里直接操作没问题的
	p_tmp = pos->second;
	m_timerQueuemap.erase(pos);
	--m_cur_size_;
	return p_tmp;
}



//如果m_ifTimeOutKick开启，就找一个超时事件，把他删除，没有释放内存，后续在别处调用zdClosesocketProc释放内存
//如果m_ifTimeOutKick不开启，就找一个超时事件，把他删除，然后新插入一个更新过时间的新节点
LPSTRUC_MSG_HEADER CSocekt::GetOverTimeTimer(time_t cur_time)
{	
	CMemory &memory = CMemory::GetInstance();
	LPSTRUC_MSG_HEADER ptmp;

	if (m_cur_size_ == 0 || m_timerQueuemap.empty())
		return NULL; //队列为空

	time_t earliesttime = GetEarliestTime(); //到multimap中去查询
	if (earliesttime <= cur_time)   //当前时间大于超时时间点
	{
		ptmp = RemoveFirstTimer();    //把这个超时的节点从 m_timerQueuemap 删掉，并把这个节点的第二项返回来；
		if(m_ifTimeOutKick != 1)  
		{
			//因为下次超时的时间我们也依然要判断，所以还要把这个节点加回来   
			time_t newinqueutime = cur_time+(m_iWaitTime);
			LPSTRUC_MSG_HEADER tmpMsgHeader = (LPSTRUC_MSG_HEADER)memory.AllocMemory(sizeof(STRUC_MSG_HEADER),false);
			tmpMsgHeader->pConn = ptmp->pConn;
			tmpMsgHeader->iCurrsequence = ptmp->iCurrsequence;			
			m_timerQueuemap.insert(std::make_pair(newinqueutime,tmpMsgHeader)); //自动排序 小->大			
			m_cur_size_++;       
		}

		if(m_cur_size_ > 0) //这个判断条件必要，因为以后我们可能在这里扩充别的代码
		{
			m_timer_value_ = GetEarliestTime(); //计时队列头部时间值保存到m_timer_value_里
		}
		return ptmp;
	}
	return NULL;
}



//把指定用户tcp连接从timer表中删除，并释放内存
void CSocekt::DeleteFromTimerQueue(lpngx_connection_t pConn)
{
    std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos,posend;
	CMemory &memory = CMemory::GetInstance();
    CLock lock(&m_timequeueMutex);

lblMTQM:
	pos    = m_timerQueuemap.begin();
	posend = m_timerQueuemap.end();
	for(; pos != posend; ++pos)	
	{
		if(pos->second->pConn == pConn)
		{			
			memory.FreeMemory(pos->second);  //释放内存
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
void CSocekt::clearAllFromTimerQueue()
{	
	std::multimap<time_t, LPSTRUC_MSG_HEADER>::iterator pos,posend;

	CMemory &memory = CMemory::GetInstance();	
	pos    = m_timerQueuemap.begin();
	posend = m_timerQueuemap.end();    
	for(; pos != posend; ++pos)	
	{
		memory.FreeMemory(pos->second);		
		--m_cur_size_; 		
	}
	m_timerQueuemap.clear();
}



//先调用GetOverTimeTimer将过了一个waittime的节点放入检测链表，再调procPingTimeOutChecking处理这些节点
//如果定时踢人开启直接踢人，否则检测心跳包是否按时发送再决定是否踢人
void* CSocekt::ServerTimerQueueMonitorThread(void* threadData)
{
    ThreadItem *pThread = static_cast<ThreadItem*>(threadData);
    CSocekt *pSocketObj = pThread->_pThis;

    time_t absolute_time,cur_time;
    int err;

    while(g_stopEvent == 0) //不退出
    {
        //这里没互斥判断，所以只是个初级判断
		if(pSocketObj->m_cur_size_ > 0)
        {
            absolute_time = pSocketObj->m_timer_value_; //省了个互斥
            cur_time = time(NULL);
            if(absolute_time < cur_time)  //当前时间大于超时时间
            {
                std::list<LPSTRUC_MSG_HEADER> m_lsIdleList; //保存要处理的内容
                LPSTRUC_MSG_HEADER result;

                err = pthread_mutex_lock(&pSocketObj->m_timequeueMutex);  
                if(err != 0) ngx_log_stderr(err,"CSocekt::ServerTimerQueueMonitorThread()中pthread_mutex_lock()失败，返回的错误码为%d!",err);
                while ((result = pSocketObj->GetOverTimeTimer(cur_time)) != NULL) 
				{
					m_lsIdleList.push_back(result);    //超时节点放入链表
				}
                err = pthread_mutex_unlock(&pSocketObj->m_timequeueMutex); 
                if(err != 0)  ngx_log_stderr(err,"CSocekt::ServerTimerQueueMonitorThread()pthread_mutex_unlock()失败，返回的错误码为%d!",err);
                LPSTRUC_MSG_HEADER tmpmsg;
                while(!m_lsIdleList.empty())
                {
                    tmpmsg = m_lsIdleList.front();
					m_lsIdleList.pop_front(); 
                    pSocketObj->procPingTimeOutChecking(tmpmsg,cur_time); 
					//这里需要检查心跳超时问题（里面的节点都是经过了一个waittime的，需要被检测）
                }
            }
        }
        
        usleep(500 * 1000); //为简化问题，我们直接每次休息500毫秒
    } 

    return (void*)0;
}


//心跳包检测时间到，该去检测心跳包是否超时的事宜，子类应该重新事先该函数以实现具体的判断动作
void CSocekt::procPingTimeOutChecking(LPSTRUC_MSG_HEADER tmpmsg,time_t cur_time)
{
	ngx_log_stderr(0,"父类procPingTimeOutChecking函数被调用！!");
/*	CMemory &memory = CMemory::GetInstance();
	memory.FreeMemory(tmpmsg);  */  
}


