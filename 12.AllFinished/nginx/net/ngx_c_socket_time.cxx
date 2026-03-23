
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
#include "ngx_c_memory.h"

//make_pair(futtime,tmpMsgHeader)   new消息头放入multimap，存连接和时间
void CSocekt::AddToTimerQueue(ngx_connection_sp pConn)
{
	time_t futtime = time(nullptr) + m_iWaitTime;

	// 互斥保护时间队列（用 std::lock_guard；如果你仍在用 CLock，也可替换回去）
	std::lock_guard<std::mutex> lk(m_timequeueMutex);

	// 分配并填充“消息头” (智能指针自动释放)
	auto tmpMsgHeader = std::make_unique<STRUC_MSG_HEADER>();
	tmpMsgHeader->pConn = pConn;                 // weak_ptr <- shared_ptr
	tmpMsgHeader->iCurrsequence = pConn->iCurrsequence;

	// 放入时间队列（注意要 move unique_ptr）
	m_timerQueuemap.emplace(futtime, std::move(tmpMsgHeader));
	++m_cur_size_;

	// 维护最早时间
	m_timer_value_ = GetEarliestTime();
}




//从multimap中取得最早的时间返回去,储存在m_timer_value_
time_t CSocekt::GetEarliestTime()
{
    std::multimap<time_t, std::unique_ptr<STRUC_MSG_HEADER>>::iterator pos;
	pos = m_timerQueuemap.begin();		
	return pos->first;	
}



//从m_timeQueuemap移除最早的时间，并把最早这个时间所在的项的消息头指针返回
std::unique_ptr<STRUC_MSG_HEADER> CSocekt::RemoveFirstTimer()
{
	std::multimap<time_t, std::unique_ptr<STRUC_MSG_HEADER>>::iterator pos;
	std::unique_ptr<STRUC_MSG_HEADER> p_tmp;
	if(m_cur_size_ <= 0)
	{
		return nullptr;
	}
	pos = m_timerQueuemap.begin(); //调用者负责互斥的，这里直接操作没问题的
	p_tmp = std::move(pos->second);
	m_timerQueuemap.erase(pos);
	--m_cur_size_;
	return p_tmp;
}



//如果m_ifTimeOutKick开启，就找一个超时事件，把他删除，没有释放内存，后续在别处调用zdClosesocketProc释放内存
//如果m_ifTimeOutKick不开启，就找一个超时事件，把他删除，然后新插入一个更新过时间的新节点
std::unique_ptr<STRUC_MSG_HEADER> CSocekt::GetOverTimeTimer(time_t cur_time)
{
	std::unique_ptr<STRUC_MSG_HEADER> ptmp;

	if (m_cur_size_ == 0 || m_timerQueuemap.empty())
		return nullptr;

	time_t earliesttime = GetEarliestTime();
	if (earliesttime <= cur_time)
	{
		ptmp = RemoveFirstTimer();

		if (m_ifTimeOutKick != 1)
		{
			time_t newinqueutime = cur_time + m_iWaitTime;

			// 直接创建对象，不使用内存池
			auto tmpMsgHeader = std::make_unique<STRUC_MSG_HEADER>();
			tmpMsgHeader->pConn = ptmp->pConn;
			tmpMsgHeader->iCurrsequence = ptmp->iCurrsequence;

			m_timerQueuemap.insert(std::make_pair(newinqueutime, std::move(tmpMsgHeader)));
			m_cur_size_++;
		}

		if (m_cur_size_ > 0)
		{
			m_timer_value_ = GetEarliestTime();
		}
		return ptmp;
	}
	return nullptr;
}



//把指定用户tcp连接从timer表中删除，并释放内存
void CSocekt::DeleteFromTimerQueue(ngx_connection_sp pConn)
{
	std::lock_guard<std::mutex> lock(m_timequeueMutex);

	auto pos = m_timerQueuemap.begin();
	while (pos != m_timerQueuemap.end())
	{
		// 尝试从 weak_ptr 转换为 shared_ptr
		auto pConnShared = pos->second->pConn.lock();
		if (pConnShared && pConnShared == pConn)  // 如果有效且相等
		{
			m_timerQueuemap.erase(pos);  // 删除元素
			--m_cur_size_;  // 更新队列大小
			pos = m_timerQueuemap.begin();  // 重新开始遍历
		}
		else
		{
			++pos;
		}
	}

	if (m_cur_size_ > 0)
	{
		m_timer_value_ = GetEarliestTime();
	}
}



//清理时间队列中所有内容
void CSocekt::clearAllFromTimerQueue()
{
	std::lock_guard<std::mutex> lock(m_timequeueMutex);  // 使用 std::lock_guard 来管理锁

	m_timerQueuemap.clear();  // 清空队列，智能指针会自动释放内存
	m_cur_size_ = 0;  // 重置队列的大小
}



//先调用GetOverTimeTimer将过了一个waittime的节点放入检测链表，再调procPingTimeOutChecking处理这些节点
//如果定时踢人开启直接踢人，否则检测心跳包是否按时发送再决定是否踢人
void CSocekt::ServerTimerQueueMonitorLoop()
{
	using namespace std::chrono_literals;

	while(!m_stop.load(std::memory_order_acquire))
    {
		bool hasWork = false;
		time_t absolute_time = 0;
		time_t now = time(nullptr);

		// 只读地看一眼是否有待处理（减少持锁时间）
		{
			std::lock_guard<std::mutex> lk(m_timequeueMutex);
			if (m_cur_size_ > 0)
			{
				absolute_time = m_timer_value_;
				hasWork = (absolute_time < now);
			}
		}

		if (hasWork)
		{
			// 把“到期的节点”批量取出来到本地，再解锁处理，减少锁竞争
			std::vector<LPSTRUC_MSG_HEADER> overdue;
			overdue.reserve(32);

			{
				std::lock_guard<std::mutex> lk(m_timequeueMutex);
				while (true)
				{
					auto item = GetOverTimeTimer(now);  // 返回 LPSTRUC_MSG_HEADER（unique_ptr）
					if (!item)
						break;
					overdue.emplace_back(std::move(item));
				}
			}

			// 逐个处理心跳超时检查（这里不需要锁）
			for (auto& tmp : overdue)
			{
				procPingTimeOutChecking(std::move(tmp), now);
			}
		}

		// 休眠 500ms（保持原语义）
		std::this_thread::sleep_for(500ms);
    } 
}

 
//心跳包检测时间到，该去检测心跳包是否超时的事宜，子类应该重新事先该函数以实现具体的判断动作
void CSocekt::procPingTimeOutChecking(LPSTRUC_MSG_HEADER tmpmsg,time_t cur_time)
{
	std::cout << "父类procPingTimeOutChecking函数被调用！!" << std::endl;
}


