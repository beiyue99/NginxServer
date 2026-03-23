
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
    iThrowsendCount = 0;                          
}



//---------------------------------------------------------------
//创建连接，放入连接链表（连接池）
void CSocekt::initconnection()
{
    CMemory &memory = CMemory::GetInstance();   

    int ilenconnpool = sizeof(ngx_connection_s);
    for(int i = 0; i < m_worker_connections; ++i)
    {
        // ❌ 错误：使用共享内存
        //auto buffer = memory.AllocBuffer(ilenconnpool, true);  // 获取原始内存块
        //auto p_Conn = new (buffer.get()) ngx_connection_s();   // 在分配的内存上构造对象
        //// 用 shared_ptr 来管理该对象
        //std::shared_ptr<ngx_connection_s> pConn(p_Conn);

         // ✅ 正确：每个连接对象独立分配
        auto pConn = std::make_shared<ngx_connection_s>();


        pConn->GetOneToUse(); //初始化连接
        m_connectionList.push_back(pConn);     //所有链接【不管是否空闲】都放在这个list
        m_freeconnectionList.push_back(pConn); //空闲连接会放在这个list
    } 
    m_free_connection_n = m_total_connection_n = m_connectionList.size(); //开始这两个列表一样大
}

//最终回收连接池，释放内存
void CSocekt::clearconnection()
{
	while(!m_connectionList.empty())
	{
		m_connectionList.pop_front(); 
	}
}



//返回一个连接，与传参的套间字绑定
ngx_connection_sp CSocekt::ngx_get_connection(int isock)
{
    ngx_connection_sp pConn;

    {
        std::lock_guard<std::mutex> lk(m_connectionMutex);

        if (!m_freeconnectionList.empty())
        {
            pConn = std::move(m_freeconnectionList.front());
            m_freeconnectionList.pop_front();
            --m_free_connection_n;
        }
        else
        {
            printf("m_freeconnectionList is Empty!\n");
            // 创建新的连接对象，并记入总表
            pConn = std::make_shared<ngx_connection_s>();
            m_connectionList.push_back(pConn);
            ++m_total_connection_n;
        }
    }

    // 初始化连接的运行态
    pConn->GetOneToUse();
    pConn->fd = isock;
    return pConn;
}



//将一个连接回收，放入空闲连接链表m_freeconnectionList，被ServerRecyConnectionThread线程函数调用
void CSocekt::ngx_free_connection(ngx_connection_sp pConn)
{
    if (!pConn)return;
    pConn->PutOneToFree();  //释放连接所持有的空间资源
    {
        std::lock_guard<std::mutex> lk(m_connectionMutex);
        m_freeconnectionList.push_back(std::move(pConn));
        ++m_free_connection_n;
    }
    // 后续可以加一个条件变量，后面“等待可用连接”的地方，可以考虑 notify_one()
}


//把连接放入m_recyconnectionList回收队列，被zdClosesocketProc函数调用
void CSocekt::inRecyConnectQueue(ngx_connection_sp pConn)
{
    if (!pConn) return;

    {
        std::lock_guard<std::mutex> lk(m_recyconnqueueMutex);

        // 查重：shared_ptr 的相等比较基于同一控制块，能直接比较
        auto it = std::find(m_recyconnectionList.begin(),
            m_recyconnectionList.end(),
            pConn);
        if (it != m_recyconnectionList.end()) {
            return; // 已在回收队列里，避免重复加入
        }

        pConn->inRecyTime = std::time(nullptr); // 记录回收时间
        ++pConn->iCurrsequence;                 // 使现有包失效（序号自增）
        m_recyconnectionList.push_back(std::move(pConn));
        ++m_totol_recyconnection_n;
    }
}



//处理连接回收的线程，如果到时间了，调用ngx_free_connection回收连接
void CSocekt::ServerRecyConnectionLoop()
{
    using namespace std::chrono_literals;
    while(!m_stop.load(std::memory_order_acquire))
    {
        std::this_thread::sleep_for(200ms);  // 原来 usleep(200*1000)
        // 快速路径：没有待回收连接
        if (m_totol_recyconnection_n <= 0)
            continue;

        const time_t now = time(nullptr);

        // 扫描回收队列
        {
            std::lock_guard<std::mutex> lk(m_recyconnqueueMutex);

            for (auto it = m_recyconnectionList.begin(); it != m_recyconnectionList.end(); /*手动前进*/)
            {
                ngx_connection_sp p_Conn = *it;
                // “未到回收时间” 且 “未停止” → 跳过
                if ((p_Conn->inRecyTime + m_RecyConnectionWaitTime) > now && !m_stop.load())
                {
                    ++it;
                    continue;
                }
                if(p_Conn->iThrowsendCount > 0)
                {
                    std::cout << "CSocekt::ServerRecyConnectionThread()中到释放时间却发现iThrowsendCount!=0" << std::endl;
                }

                it = m_recyconnectionList.erase(it);
                --m_totol_recyconnection_n;
                ngx_free_connection(p_Conn);
            }
        } 
        if(m_stop.load())
        {
            std::lock_guard<std::mutex> lk(m_recyconnqueueMutex);
            for (auto it = m_recyconnectionList.begin(); it != m_recyconnectionList.end(); /* */)
            {
                ngx_connection_sp p_Conn = *it;
                it = m_recyconnectionList.erase(it);
                --m_totol_recyconnection_n;
                ngx_free_connection(p_Conn);
            }
            break;
        }
    }  
}

void CSocekt::ngx_close_connection(ngx_connection_sp pConn)
{    
    ngx_free_connection(pConn); 
    //将一个连接回收，放入空闲连接链表m_freeconnectionList
    if(pConn->fd != -1)
    {
        close(pConn->fd);
        pConn->fd = -1;
    }    
    // 释放 weak_ptr
    if (pConn->epoll_weak_ptr) {
        delete static_cast<std::weak_ptr<ngx_connection_s>*>(pConn->epoll_weak_ptr);
        pConn->epoll_weak_ptr = nullptr;
    }
}
