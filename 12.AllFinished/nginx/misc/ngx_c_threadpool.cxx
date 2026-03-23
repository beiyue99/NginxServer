
#include <stdarg.h>
#include <unistd.h>  //usleep

#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_threadpool.h"
#include "ngx_c_memory.h"
#include "ngx_macro.h"
#include <iostream>

//构造函数
CThreadPool::CThreadPool()
    : m_iRunningThreadNum(0), m_iLastEmgTime(0), m_iRecvMsgQueueCount(0), m_shutdown(false)
{
}
//析构函数
CThreadPool::~CThreadPool()
{    
    clearMsgRecvQueue();
}

void CThreadPool::clearMsgRecvQueue()
{
	while(!m_MsgRecvQueue.empty())
	{
		m_MsgRecvQueue.pop_front(); 
	}	
}

//线程池创建threadNum数量的线程，线程入口函数是ThreadFunc
bool CThreadPool::Create(int threadNum)
{
    m_iThreadNum = threadNum; //保存要创建的线程数量
    // 创建线程
    for (int i = 0; i < m_iThreadNum; ++i)
    {
        auto pNew = std::make_shared<ThreadItem>(this);  // 使用 shared_ptr 管理线程项
        m_threadVector.push_back(pNew);
        // 启动线程
        try
        {
            // 捕获 self 而不是创建新的 shared_ptr
            pNew->_Handle = std::thread([this, threadIndex = i]() {
                ThreadFunc(threadIndex);
                });
        }
        catch (const std::exception& e)
        {
            std::cout << "CThreadPool::Create() 创建线程失败，异常: " << e.what() << std::endl;
            return false;
        }
    }



    for (auto it = m_threadVector.begin(); it != m_threadVector.end(); it++)
    {
        if ((*it)->ifrunning == false) //这个条件保证所有线程完全启动起来，以保证整个线程池中的线程正常工作；
        {
            usleep(100 * 1000);
        }
    }
    return true;
}



//线程入口函数，当用std::thread创建线程后，这个ThreadFunc()函数都会被立即执行；
void CThreadPool::ThreadFunc(int threadIndex)
{
    try
    {
        while (true)
        {
            std::unique_ptr<char[]> jobbuf = nullptr;  // 将 jobbuf 定义为 std::unique_ptr<char[]>

            // 1. 用代码块限制锁的作用域
            {
                std::unique_lock<std::mutex> lock(m_pthreadMutex);

                // 2. 标记线程为空闲状态
                m_threadVector[threadIndex]->ifrunning = false;

                // 3. 等待消息或退出信号
                m_pthreadCond.wait(lock, [this]() {
                    return !m_MsgRecvQueue.empty() || m_shutdown;
                    });

                // 4. 检查是否需要退出
                if (m_shutdown)
                {
                    break;  // 退出线程
                }

                // 5. 获取消息
                jobbuf = std::move(m_MsgRecvQueue.front());  // 使用 move 赋值
                m_MsgRecvQueue.pop_front();
                --m_iRecvMsgQueueCount;

                // 6. 标记线程为运行状态
                m_threadVector[threadIndex]->ifrunning = true;
                ++m_iRunningThreadNum;

            } // 锁在这里自动释放

            // 7. 在锁外处理消息（允许其他线程并发工作）
            //g_socket.threadRecvProcFunc(jobbuf);
            g_socket.threadRecvProcFunc(jobbuf.get());  // 获取裸指针进行处理
            // 8. 更新运行状态
            {
                std::unique_lock<std::mutex> lock(m_pthreadMutex);
                --m_iRunningThreadNum;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cout << "CThreadPool::ThreadFunc 中捕获到异常: " << e.what() << std::endl;
    }
}

//停止所有线程【等待结束线程池中所有线程，该函数返回后，应该是所有线程池中线程都结束了】
void CThreadPool::StopAll()
{
    if (m_shutdown)
    {
        return;
    }
    m_shutdown = true;

    //唤醒等待条件变量的所有线程
    {
        std::lock_guard<std::mutex> lock(m_pthreadMutex);
        m_pthreadCond.notify_all();  // 唤醒所有线程
    }

    // 等待所有线程结束
    for (auto& pThread : m_threadVector)
    {
        if (pThread->_Handle.joinable())
        {
            pThread->_Handle.join();  // 等待线程结束
        }
    }
    m_threadVector.clear();
    std::cout << "CThreadPool::StopAll() 线程池中的线程全部正常结束!" << std::endl;
}



//入消息队列，并调用Call触发线程池中线程来处理该消息，该函数被ngx_wait_request_handler_proc_plast调用
void CThreadPool::inMsgRecvQueueAndSignal(BufferPtr buf)
{
    {
        std::lock_guard<std::mutex> lock(m_pthreadMutex);  // 加锁
        m_MsgRecvQueue.push_back(std::move(buf));                       // 入消息队列
        ++m_iRecvMsgQueueCount;                              // 收消息队列数字+1
    }

    Call();  // 调用线程池中的线程来处理消息
}


//调用p通知一个线程池中的线程下来干活
void CThreadPool::Call()
{
    {
        std::lock_guard<std::mutex> lock(m_pthreadMutex);
        m_pthreadCond.notify_one();  // 唤醒一个线程
    }
     
    if (m_iThreadNum == m_iRunningThreadNum)
    {
        time_t currtime = time(NULL);
        if (currtime - m_iLastEmgTime > 10)  // 最少间隔10秒钟才报一次线程池中线程不够用的问题；
        {
            m_iLastEmgTime = currtime;  // 更新时间
            std::cout << "CThreadPool::Call() 中发现线程池中当前空闲线程数量为0，要考虑扩容线程池了!" << std::endl;
        }
    }
}

