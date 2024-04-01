
//和 线程池 有关的函数放这里
/*
公众号：程序员速成     q群：716480601
王健伟老师 《Linux C++通讯架构实战》
商业级质量的代码，完整的项目，帮你提薪至少10K
*/

#include <stdarg.h>
#include <unistd.h>  //usleep

#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_threadpool.h"
#include "ngx_c_memory.h"
#include "ngx_macro.h"

//静态成员初始化
pthread_mutex_t CThreadPool::m_pthreadMutex = PTHREAD_MUTEX_INITIALIZER;  //#define PTHREAD_MUTEX_INITIALIZER ((pthread_mutex_t) -1)
pthread_cond_t CThreadPool::m_pthreadCond = PTHREAD_COND_INITIALIZER;     //#define PTHREAD_COND_INITIALIZER ((pthread_cond_t) -1)
bool CThreadPool::m_shutdown = false;    //刚开始标记整个线程池的线程是不退出的      

//构造函数
CThreadPool::CThreadPool()
{
    m_iRunningThreadNum = 0;  //正在运行的线程，开始给个0【注意这种写法：原子的对象给0也可以直接赋值，当整型变量来用】
    m_iLastEmgTime = 0;       //上次报告线程不够用了的时间；
    m_iRecvMsgQueueCount = 0; //收消息队列
}

//析构函数
CThreadPool::~CThreadPool()
{    
    clearMsgRecvQueue();
}

//各种清理函数-------------------------
void CThreadPool::clearMsgRecvQueue()
{
	char * sTmpMempoint;
	CMemory *p_memory = CMemory::GetInstance();

	while(!m_MsgRecvQueue.empty())
	{
		sTmpMempoint = m_MsgRecvQueue.front();		
		m_MsgRecvQueue.pop_front(); 
		p_memory->FreeMemory(sTmpMempoint);
	}	
}

//创建线程池中的线程，设置ThreadFunc为线程入口函数
bool CThreadPool::Create(int threadNum)
{    
    ThreadItem *pNew;
    int err;

    m_iThreadNum = threadNum; //保存要创建的线程数量    
    
    for(int i = 0; i < m_iThreadNum; ++i)
    {
        m_threadVector.push_back(pNew = new ThreadItem(this));             //创建 一个新线程对象 并入到容器中         
        err = pthread_create(&pNew->_Handle, NULL, ThreadFunc, pNew);      //创建线程，错误不返回到errno，一般返回错误码
        if(err != 0)
        {
            //创建线程有错
            ngx_log_stderr(err,"CThreadPool::Create()创建线程%d失败，返回的错误码为%d!",i,err);
            return false;
        }
        else
        {
            //创建线程成功
        }        
    } 

    //我们必须保证每个线程都启动并运行到pthread_cond_wait()，本函数才返回，只有这样，这几个线程才能进行后续的正常工作 
    std::vector<ThreadItem*>::iterator iter;
lblfor:
    for(iter = m_threadVector.begin(); iter != m_threadVector.end(); iter++)
    {
        if( (*iter)->ifrunning == false) //这个条件保证所有线程完全启动起来，以保证整个线程池中的线程正常工作；
        {
            //这说明有没有启动完全的线程
            usleep(100 * 1000);  //单位是微妙,又因为1毫秒=1000微妙，所以 100 *1000 = 100毫秒
            goto lblfor;
        }
    }
    return true;
}




//线程的入口函数，卡在pthread_cond_wait，唤醒后调用threadRecvProcFunc处理消息
void* CThreadPool::ThreadFunc(void* threadData)
{
    //这个是静态成员函数，是不存在this指针的；
    ThreadItem *pThread = static_cast<ThreadItem*>(threadData);
    CThreadPool *pThreadPoolObj = pThread->_pThis;
    
    CMemory *p_memory = CMemory::GetInstance();	    
    int err;

    pthread_t tid = pthread_self(); //获取线程自身id，以方便调试打印信息等    
    while(true)
    {
        err = pthread_mutex_lock(&m_pthreadMutex);  
        if(err != 0) ngx_log_stderr(err,"CThreadPool::ThreadFunc()中pthread_mutex_lock()失败，返回的错误码为%d!",err);//有问题，要及时报告
        while ( (pThreadPoolObj->m_MsgRecvQueue.size() == 0) && m_shutdown == false)
        {
            if(pThread->ifrunning == false)            
                pThread->ifrunning = true; 
            pthread_cond_wait(&m_pthreadCond, &m_pthreadMutex); //整个服务器程序刚初始化的时候，所有线程必然是卡在这里等待的；
        }

        if(m_shutdown)
        {   
            pthread_mutex_unlock(&m_pthreadMutex); //解锁互斥量
            break;                     
        }

        //走到这里，可以取得消息进行处理了【消息队列中必然有消息】,注意，目前还是互斥着呢
        char *jobbuf = pThreadPoolObj->m_MsgRecvQueue.front();     //返回第一个元素但不检查元素存在与否
        pThreadPoolObj->m_MsgRecvQueue.pop_front();                //移除第一个元素但不返回	
        --pThreadPoolObj->m_iRecvMsgQueueCount;                    //收消息队列数字-1
               
        //可以解锁互斥量了
        err = pthread_mutex_unlock(&m_pthreadMutex); 
        if(err != 0)  ngx_log_stderr(err,"CThreadPool::ThreadFunc()中pthread_mutex_unlock()失败，返回的错误码为%d!",err);//有问题，要及时报告
        
        //能走到这里的，就是有消息可以处理，开始处理
        ++pThreadPoolObj->m_iRunningThreadNum;    //原子+1【记录正在干活的线程数量增加1】，这比互斥量要快很多

        g_socket.threadRecvProcFunc(jobbuf);     //处理消息队列中来的消息

        p_memory->FreeMemory(jobbuf);              //释放消息内存 
        --pThreadPoolObj->m_iRunningThreadNum;     //原子-1【记录正在干活的线程数量减少1】

    } 

    return (void*)0;
}




//停止所有线程【等待结束线程池中所有线程，该函数返回后，应该是所有线程池中线程都结束了】
void CThreadPool::StopAll() 
{
    //(1)已经调用过，就不要重复调用了
    if(m_shutdown == true)
    {
        return;
    }
    m_shutdown = true;

    //(2)唤醒等待该条件【卡在pthread_cond_wait()的】的所有线程，一定要在改变条件状态以后再给线程发信号
    int err = pthread_cond_broadcast(&m_pthreadCond); 
    if(err != 0)
    {
        //这肯定是有问题，要打印紧急日志
        ngx_log_stderr(err,"CThreadPool::StopAll()中pthread_cond_broadcast()失败，返回的错误码为%d!",err);
        return;
    }

    //(3)等等线程，让线程真返回    
    std::vector<ThreadItem*>::iterator iter;
	for(iter = m_threadVector.begin(); iter != m_threadVector.end(); iter++)
    {
        pthread_join((*iter)->_Handle, NULL); //等待一个线程终止
    }

    //流程走到这里，那么所有的线程池中的线程肯定都返回了；
    pthread_mutex_destroy(&m_pthreadMutex);
    pthread_cond_destroy(&m_pthreadCond);    

    //(4)释放一下new出来的ThreadItem【线程池中的线程】    
	for(iter = m_threadVector.begin(); iter != m_threadVector.end(); iter++)
	{
		if(*iter)
			delete *iter;
	}
	m_threadVector.clear();

    ngx_log_stderr(0,"CThreadPool::StopAll()成功返回，线程池中线程全部正常结束!");
    return;    
}



//--------------------------------------------------------------------------------------
//把消息放入消息队列，并调用Call函数触发线程池中线程来处理该消息
void CThreadPool::inMsgRecvQueueAndSignal(char *buf)
{
    //互斥
    int err = pthread_mutex_lock(&m_pthreadMutex);     
    if(err != 0)
    {
        ngx_log_stderr(err,"CThreadPool::inMsgRecvQueueAndSignal()pthread_mutex_lock()失败，返回的错误码为%d!",err);
    }
        
    m_MsgRecvQueue.push_back(buf);	         //入消息队列
    ++m_iRecvMsgQueueCount;                  //收消息队列数字+1，个人认为用变量更方便一点，比 m_MsgRecvQueue.size()高效

    //取消互斥
    err = pthread_mutex_unlock(&m_pthreadMutex);   
    if(err != 0)
    {
        ngx_log_stderr(err,"CThreadPool::inMsgRecvQueueAndSignal()pthread_mutex_unlock()失败，返回的错误码为%d!",err);
    }

    //可以激发一个线程来干活了
    Call();                                  
    return;
}



//来任务了，调一个线程池中的线程下来干活
void CThreadPool::Call()
{
    int err = pthread_cond_signal(&m_pthreadCond); //唤醒一个等待该条件的线程，也就是可以唤醒卡在pthread_cond_wait()的线程
    if(err != 0 )
    {
        //这是有问题啊，要打印日志啊
        ngx_log_stderr(err,"CThreadPool::Call()中pthread_cond_signal()失败，返回的错误码为%d!",err);
    }
    if(m_iThreadNum == m_iRunningThreadNum) //线程池中线程总量，跟当前正在干活的线程数量一样，说明所有线程都忙碌起来，线程不够用了
    {        
        time_t currtime = time(NULL);
        if(currtime - m_iLastEmgTime > 10) //最少间隔10秒钟才报一次线程池中线程不够用的问题；
        {
            m_iLastEmgTime = currtime;  //更新时间
            ngx_log_stderr(0,"CThreadPool::Call()中发现线程池中当前空闲线程数量为0，要考虑扩容线程池了!");
        }
    } //end if 

/*
    //-------------------------------------------------------如下内容都是一些测试代码；
    //唤醒丢失？--------------------------------------------------------------------------
    //(2)整个工程中，只在一个线程（主线程）中调用了Call，所以不存在多个线程调用Call的情形。
    if(ifallthreadbusy == false)
    {
        //有空闲线程  ，有没有可能我这里调用   pthread_cond_signal()，但因为某个时刻线程曾经全忙过，导致本次调用 pthread_cond_signal()并没有激发某个线程的pthread_cond_wait()执行呢？
           //我认为这种可能性不排除，这叫 唤醒丢失。如果真出现这种问题，我们如何弥补？
        if(irmqc > 5) //我随便来个数字比如给个5吧
        {
            //如果有空闲线程，并且 接收消息队列中超过5条信息没有被处理，则我总感觉可能真的是 唤醒丢失
            //唤醒如果真丢失，我是否考虑这里多唤醒一次？以尝试逐渐补偿回丢失的唤醒？此法是否可行，我尚不可知，我打印一条日志【其实后来仔细相同：唤醒如果真丢失，也无所谓，因为ThreadFunc()会一直处理直到整个消息队列为空】
            ngx_log_stderr(0,"CThreadPool::Call()中感觉有唤醒丢失发生，irmqc = %d!",irmqc);

            int err = pthread_cond_signal(&m_pthreadCond); //唤醒一个等待该条件的线程，也就是可以唤醒卡在pthread_cond_wait()的线程
            if(err != 0 )
            {
                //这是有问题啊，要打印日志啊
                ngx_log_stderr(err,"CThreadPool::Call()中pthread_cond_signal 2()失败，返回的错误码为%d!",err);
            }
        }
    }  //end if

    //(3)准备打印一些参考信息【10秒打印一次】,当然是有触发本函数的情况下才行
    m_iCurrTime = time(NULL);
    if(m_iCurrTime - m_iPrintInfoTime > 10)
    {
        m_iPrintInfoTime = m_iCurrTime;
        int irunn = m_iRunningThreadNum;
        ngx_log_stderr(0,"信息：当前消息队列中的消息数为%d,整个线程池中线程数量为%d,正在运行的线程数量为 = %d!",irmqc,m_iThreadNum,irunn); //正常消息，三个数字为 1，X，0
    }
    */
    return;
}
//唤醒丢失问题，sem_t sem_write;
//参考信号量解决方案：https://blog.csdn.net/yusiguyuan/article/details/20215591  linux多线程编程--信号量和条件变量 唤醒丢失事件
