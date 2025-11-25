
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

//--------------------------------------------------------------------------
//构造函数
CSocekt::CSocekt()
{
    //配置相关
    m_worker_connections = 1;      //epoll连接最大项数
    m_ListenPortCount = 1;         //监听一个端口
    m_RecyConnectionWaitTime = 60; //等待这么些秒后才回收连接

    //epoll相关
    m_epollhandle = -1;          //epoll返回的句柄

    m_iLenPkgHeader = sizeof(COMM_PKG_HEADER);    //包头的sizeof值【占用的字节数】
    m_iLenMsgHeader =  sizeof(STRUC_MSG_HEADER);  //消息头的sizeof值【占用的字节数】


    //各种队列相关
    m_iSendMsgQueueCount     = 0;     //发消息队列大小
    m_totol_recyconnection_n = 0;     //待释放连接队列大小
    m_cur_size_              = 0;     //当前计时队列尺寸
    m_timer_value_           = 0;     //当前计时队列头部的时间值

    //在线用户相关
    m_onlineUserCount        = 0;     //在线用户数量统计，先给0  
    return;	
}



//调用ngx_open_listening_sockets 创建socket，打开监听端口，  放入监听队列
bool CSocekt::Initialize()
{
    ReadConf();  //读配置项
    if(ngx_open_listening_sockets() == false)  
        return false;  
    return true;
}

//初始化锁和信号量，创建三大线程：发数据线程、回收连接线程、处理不发心跳包用户线程
bool CSocekt::Initialize_subproc()
{
    // 发送队列线程
    auto pSend = std::make_unique<ThreadItem>(this);
    m_threadVector.push_back(std::move(pSend));
    m_threadVector.back()->_Handle = std::thread([this]() {
        ServerSendQueueLoop();
        });

    // 回收连接线程
    auto pRecy = std::make_unique<ThreadItem>(this);
    m_threadVector.push_back(std::move(pRecy));
    m_threadVector.back()->_Handle = std::thread([this]() {
        ServerRecyConnectionLoop();
        });

    // 心跳监控线程（根据开关）
    if (m_ifkickTimeCount == 1) {
        auto pTimer = std::make_unique<ThreadItem>(this);
        m_threadVector.push_back(std::move(pTimer));
        m_threadVector.back()->_Handle = std::thread([this]() {
            ServerTimerQueueMonitorLoop();
            });
    }
    return true;
}






//--------------------------------------------------------------------------
//释放函数
CSocekt::~CSocekt()
{
    try {
        std::vector<ngx_listening_s>::iterator pos;
        m_ListenSocketList.clear();
    }
    catch (const std::exception& e) {
        ngx_log_stderr(0, "Exception caught in CSocekt destructor: %s", e.what());
    }
}

void CSocekt::Shutdown_subproc()
{
    m_stop.store(true, std::memory_order_release);
    // 通知 ServerSendQueueThread() 可以继续执行
    m_sendMessageQueueCv.notify_all();
    // 等待所有线程终止
    for (auto& threadItem : m_threadVector)
    {
        if (threadItem && threadItem->_Handle.joinable())
        {
            threadItem->_Handle.join();  // 等待线程终止
        }
    }
	m_threadVector.clear();
    clearMsgSendQueue();
    clearconnection();
    clearAllFromTimerQueue();
}

//清理TCP发送消息队列
void CSocekt::clearMsgSendQueue()
{
	while(!m_MsgSendQueue.empty())
	{
		m_MsgSendQueue.pop_front(); 
	}	
}

//专门用于读各种配置项
void CSocekt::ReadConf()
{
    CConfig *p_config = CConfig::GetInstance();
    m_worker_connections      = p_config->GetIntDefault("worker_connections",m_worker_connections);              //epoll连接的最大项数
    m_ListenPortCount         = p_config->GetIntDefault("ListenPortCount",m_ListenPortCount);                    //取得要监听的端口数量
    m_RecyConnectionWaitTime  = p_config->GetIntDefault("Sock_RecyConnectionWaitTime",m_RecyConnectionWaitTime); //等待这么些秒后才回收连接

    m_ifkickTimeCount         = p_config->GetIntDefault("Sock_WaitTimeEnable",0);                                //是否开启踢人时钟，1：开启   0：不开启
	m_iWaitTime               = p_config->GetIntDefault("Sock_MaxWaitTime",m_iWaitTime);                         //多少秒检测一次是否 心跳超时，只有当Sock_WaitTimeEnable = 1时，本项才有用	
	m_iWaitTime               = (m_iWaitTime > 5)?m_iWaitTime:5;                                                 //不建议低于5秒钟，因为无需太频繁
    m_ifTimeOutKick           = p_config->GetIntDefault("Sock_TimeOutKick",0);                                   //当时间到达Sock_MaxWaitTime指定的时间时，直接把客户端踢出去，只有当Sock_WaitTimeEnable = 1时，本项才有用 

    m_floodAkEnable          = p_config->GetIntDefault("Sock_FloodAttackKickEnable",0);                          //Flood攻击检测是否开启,1：开启   0：不开启
	m_floodTimeInterval      = p_config->GetIntDefault("Sock_FloodTimeInterval",100);                            //表示每次收到数据包的时间间隔是100(毫秒)
	m_floodKickCount         = p_config->GetIntDefault("Sock_FloodKickCounter",10);                              //累积多少次踢出此人

    return;
}
//创建socket，打开监听端口，  放入监听队列
bool CSocekt::ngx_open_listening_sockets()
{    
    int                isock;                //socket
    struct sockaddr_in serv_addr;            //服务器的地址结构体
    int                iport;                //端口
    char               strinfo[100];         //临时字符串 
   
    //初始化相关
    memset(&serv_addr,0,sizeof(serv_addr));  //先初始化一下
    serv_addr.sin_family = AF_INET;                //选择协议族为IPV4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //监听本地所有的IP地址；INADDR_ANY表示的是一个服务器上所有的网卡（服务器可能不止一个网卡）多个本地ip地址都进行绑定端口号，进行侦听。

    //中途用到一些配置信息
    CConfig *p_config = CConfig::GetInstance();
    for(int i = 0; i < m_ListenPortCount; i++) //要监听这么多个端口
    {        
        isock = socket(AF_INET,SOCK_STREAM,0); //系统函数，成功返回非负描述符，出错返回-1
        if(isock == -1)
        {
            ngx_log_stderr(errno,"CSocekt::Initialize()中socket()失败,i=%d.",i);
            return false;
        }

        int reuseaddr = 1;
        if (setsockopt(isock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1) {
            ngx_log_stderr(errno, "setsockopt(SO_REUSEADDR)失败");
            close(isock);
            return false;
        }

        //设置该socket为非阻塞
        if(setnonblocking(isock) == false)
        {                
            ngx_log_stderr(errno,"CSocekt::Initialize()中setnonblocking()失败,i=%d.",i);
            close(isock);
            return false;
        }

        //设置本服务器要监听的地址和端口，这样客户端才能连接到该地址和端口并发送数据        
        strinfo[0] = 0;
        sprintf(strinfo,"ListenPort%d",i);
        iport = p_config->GetIntDefault(strinfo,10000);
        serv_addr.sin_port = htons((in_port_t)iport);   //in_port_t其实就是uint16_t

        //绑定服务器地址结构体
        if(bind(isock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        {
            ngx_log_stderr(errno,"CSocekt::Initialize()中bind()失败,i=%d.",i);
            close(isock);
            return false;
        }
        
        //开始监听
        if(listen(isock,NGX_LISTEN_BACKLOG) == -1)
        {
            ngx_log_stderr(errno,"CSocekt::Initialize()中listen()失败,i=%d.",i);
            close(isock);
            return false;
        }

        ngx_listening_sp p_listensocketitem = std::make_shared<ngx_listening_s>();
        p_listensocketitem->port = iport;                          //记录下所监听的端口号
        p_listensocketitem->fd   = isock;                          //套接字木柄保存下来   
        ngx_log_error_core(NGX_LOG_INFO,0,"监听%d端口成功!",iport); //显示一些信息到日志中
        m_ListenSocketList.push_back(p_listensocketitem);          //加入到队列中
    }   
    if(m_ListenSocketList.size() <= 0)  
        return false;
    return true;
}

bool CSocekt::setnonblocking(int sockfd) 
{    
    int nb=1; //0：清除，1：设置  
    if(ioctl(sockfd, FIONBIO, &nb) == -1) //FIONBIO：设置/清除非阻塞I/O标记：0：清除，1：设置
    {
        return false;
    }
    return true;
}

void CSocekt::ngx_close_listening_sockets()
{
    for(int i = 0; i < m_ListenPortCount; i++) //要关闭这么多个监听端口
    {  
        close(m_ListenSocketList[i]->fd);
        ngx_log_error_core(NGX_LOG_INFO,0,"关闭监听端口%d!",m_ListenSocketList[i]->port); //显示一些信息到日志中
    }
    return;
}


void CSocekt::msgSend(BufferPtr psendbuf)
{
    std::lock_guard<std::mutex> lk(m_sendMessageQueueMutex);

    m_MsgSendQueue.push_back(std::move(psendbuf)); // unique_ptr 进队列
    ++m_iSendMsgQueueCount;

    m_sendMessageQueueCv.notify_one();
}


//调用inRecyConnectQueue放入待回收连接队列
void CSocekt::zdClosesocketProc(ngx_connection_sp p_Conn)
{
    DeleteFromTimerQueue(p_Conn);

    //关闭socket
    if(p_Conn->fd != -1)
    {   
        close(p_Conn->fd); 
        p_Conn->fd = -1;
    }

    if(p_Conn->iThrowsendCount > 0)  
        --p_Conn->iThrowsendCount;   //归0

    inRecyConnectQueue(p_Conn);
    //放入待回收连接队列
    return;
}

//测试是否flood攻击成立，成立则返回true，否则返回false
bool CSocekt::TestFlood(ngx_connection_sp pConn)
{
    struct  timeval sCurrTime;   //当前时间结构
	uint64_t        iCurrTime;   //当前时间（单位：毫秒）
	bool  reco      = false;
	
	gettimeofday(&sCurrTime, NULL); //取得当前时间
    iCurrTime =  (sCurrTime.tv_sec * 1000 + sCurrTime.tv_usec / 1000);  //毫秒
	if((iCurrTime - pConn->FloodkickLastTime) < m_floodTimeInterval)   //两次收到包的时间 < 100毫秒
	{
        //发包太频繁记录
		pConn->FloodAttackCount++;
		pConn->FloodkickLastTime = iCurrTime;
	}
	else
	{
        //既然发布不这么频繁，则恢复计数值
		pConn->FloodAttackCount = 0;
		pConn->FloodkickLastTime = iCurrTime;
	}

    ngx_log_stderr(0,"pConn->FloodAttackCount=%d,m_floodKickCount=%d.",pConn->FloodAttackCount,m_floodKickCount);

	if(pConn->FloodAttackCount >= m_floodKickCount)
	{
		//可以踢此人的标志
		reco = true;
	}
	return reco;
}


//创建epoll，创建连接，放入连接链表。在epoll添加监听的套间字事件
int CSocekt::ngx_epoll_init()
{
    m_epollhandle = epoll_create(m_worker_connections);   //直接以epoll连接的最大项数为参数，肯定是>0的； 

    if (m_epollhandle == -1) 
    {
        ngx_log_stderr(errno,"CSocekt::ngx_epoll_init()中epoll_create()失败.");
        exit(2); 
    }

    initconnection();
    int index = 0;
	for(auto& listenSocket : m_ListenSocketList)
    {
        ngx_connection_sp p_Conn = ngx_get_connection(listenSocket->fd);
        if (p_Conn == NULL)
        {
            ngx_log_stderr(errno,"CSocekt::ngx_epoll_init()中ngx_get_connection()失败.");
            exit(2); 
        }

        p_Conn->listening = listenSocket;   //连接对象 和监听对象关联，方便通过连接对象找监听对象
        listenSocket->connection = std::weak_ptr(p_Conn);  //监听对象 和连接对象关联，方便通过监听对象找连接对象

        p_Conn->rhandler = &CSocekt::ngx_event_accept;

        int ret = ngx_epoll_oper_event(
            listenSocket->fd,
            EPOLL_CTL_ADD,
            EPOLLIN | EPOLLRDHUP | EPOLLET,
            0,
            p_Conn
        );

        if (ret == -1)
        {
            ngx_log_stderr(errno, "ngx_epoll_oper_event()失败! fd=%d, port=%d",
                listenSocket->fd, listenSocket->port);
            exit(2);
        }

        index++;
    }
    return 1;
}


int CSocekt::ngx_epoll_oper_event(
    int fd, uint32_t eventtype, uint32_t flag,
    int bcaction, ngx_connection_sp pConn)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));

    if (eventtype == EPOLL_CTL_ADD) {
        ev.events = flag;
        pConn->events = flag;

        std::weak_ptr<ngx_connection_s>* wp = new std::weak_ptr<ngx_connection_s>(pConn);
        ev.data.ptr = wp;
        pConn->epoll_weak_ptr = wp;
    }
    else if (eventtype == EPOLL_CTL_MOD) {
        ev.events = pConn->events;
        if (bcaction == 0) {
            ev.events |= flag;
        }
        else if (bcaction == 1) {
            ev.events &= ~flag;
        }
        else {
            ev.events = flag;
        }
        pConn->events = ev.events;
        ev.data.ptr = pConn->epoll_weak_ptr;

        printf("  MOD: ev.data.ptr=%p\n", ev.data.ptr);
    }
    else if (eventtype == EPOLL_CTL_DEL) {
        ev.events = 0;
        pConn->events = 0;
        ev.data.ptr = nullptr;
    }

    int ret = epoll_ctl(m_epollhandle, eventtype, fd, &ev);

    if (ret == -1) {
        int saved_errno = errno;
        printf("  ❌ epoll_ctl 失败: errno=%d (%s)\n",
            saved_errno, strerror(saved_errno));

        if (eventtype == EPOLL_CTL_ADD && pConn->epoll_weak_ptr) {
            delete static_cast<std::weak_ptr<ngx_connection_s>*>(pConn->epoll_weak_ptr);
            pConn->epoll_weak_ptr = nullptr;
        }
        return -1;
    }

    return 1;
}

//调用epoll_wait返回事件并处理
int CSocekt::ngx_epoll_process_events(int timer)
{
    int events = epoll_wait(m_epollhandle, m_events, NGX_MAX_EVENTS, timer);

    if (events == -1)
    {
        if (errno == EINTR)
        {
            ngx_log_error_core(NGX_LOG_INFO, errno, "epoll_wait()失败(EINTR)!");
            return 1;
        }
        else
        {
            ngx_log_stderr(errno, "epoll_wait()失败!");
            return 0;
        }
    }

    if (events == 0)
    {
        if (timer != -1) return 1;
        ngx_log_error_core(NGX_LOG_ALERT, 0, "epoll_wait()没超时却没返回事件!");
        return 0;
    }

    // 处理事件
    for (int i = 0; i < events; ++i)
    {
        uint32_t revents = m_events[i].events;

        std::weak_ptr<ngx_connection_s>* wp =
            static_cast<std::weak_ptr<ngx_connection_s>*>(m_events[i].data.ptr);

        if (!wp) {
            continue;
        }

        ngx_connection_sp p_Conn;
        try {
            p_Conn = wp->lock();
        }
        catch (const std::exception& e) {
            continue;
        }

        if (!p_Conn) {
            continue;
        }

        printf("连接信息: fd=%d\n",p_Conn->fd);

        if (p_Conn->listening.lock()) {
            std::shared_ptr<ngx_listening_s> listening_ptr = p_Conn->listening.lock();
            printf(", port=%d", listening_ptr->port);
        }

        if (revents & EPOLLIN)
        {
            if (!p_Conn->rhandler) {
                continue;
            }
            (this->*(p_Conn->rhandler))(p_Conn);
        }

        if (revents & EPOLLOUT)
        {
            if (revents & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
            {
                printf("  客户端断开\n");
                --p_Conn->iThrowsendCount;
            }
            else
            {
                if (p_Conn->whandler) {
                    (this->*(p_Conn->whandler))(p_Conn);
                }
            }
        }
    }
    return 1;
}



//发消息队列数据的线程
void CSocekt::ServerSendQueueLoop()
{
    try
    {
        while (!m_stop.load(std::memory_order_acquire)) //不退出
        {
            CMemory::BufferPtr buf;
            STRUC_MSG_HEADER* pMsgHeader = nullptr;
            LPCOMM_PKG_HEADER pPkgHeader = nullptr;
            ngx_connection_sp p_Conn;
            {
                std::unique_lock<std::mutex> lk(m_sendMessageQueueMutex);
                m_sendMessageQueueCv.wait(lk, [&] {
                    return m_iSendMsgQueueCount > 0 || m_stop.load();
                    });

                if (m_stop.load())   break;


                auto it = m_MsgSendQueue.begin();     // begin返回的是临时对象，不能引用
                auto end = m_MsgSendQueue.end();

                while (it != end)
                {
                    auto& temp = *it;       //      temp 是智能指针：     unique_ptr<char[]>&    智能指针不能拷贝，用引用
                    char* base = temp.get();// 裸指针用于解析
                    //pos是数据包，一开始指向消息头，发送的时候发包头和包体

                    pMsgHeader = reinterpret_cast<STRUC_MSG_HEADER*>(base);
                    pPkgHeader = reinterpret_cast<LPCOMM_PKG_HEADER>(base + m_iLenMsgHeader);

                    p_Conn = pMsgHeader->pConn.lock(); // 使用 lock() 将 weak_ptr 转换为 shared_ptr
                    if (!p_Conn) {
                        it = m_MsgSendQueue.erase(it);
                        --m_iSendMsgQueueCount;
                        continue;
                    }
                    // 跳过无效消息
                    if (p_Conn->iCurrsequence != pMsgHeader->iCurrsequence) {
                        it = m_MsgSendQueue.erase(it);
                        --m_iSendMsgQueueCount;
                        continue;
                    }

                    // 发送缓冲区满了
                    if (p_Conn->iThrowsendCount > 0)
                    {
                        it++;
                        continue;
                    }

                    //走到这里，可以发送消息
                    buf = std::move(temp);  // 把 unique_ptr 从队列移到连接
                    it = m_MsgSendQueue.erase(it);  //返回下一个有效迭代器
                    --m_iSendMsgQueueCount;      //发送消息队列容量少1	
                    break;
                }
            }  // 释放锁
            if (!buf) continue;

            // 重新解析消息（因为在锁外，需要重新获取指针）
            char* base = buf.get();

            pMsgHeader = reinterpret_cast<STRUC_MSG_HEADER*>(base);
            pPkgHeader = reinterpret_cast<LPCOMM_PKG_HEADER>(base + m_iLenMsgHeader);

            p_Conn = pMsgHeader->pConn.lock();  // 使用 lock() 将 weak_ptr 转换为 shared_ptr

            if (!p_Conn) continue; // 如果连接无效，跳过

            // 转移所有权到连接对象
            p_Conn->psendMemPointer = std::move(buf);
            p_Conn->psendbuf = reinterpret_cast<char*>(pPkgHeader);
            p_Conn->isendlen = ntohs(pPkgHeader->pkgLen);;       //包头+包体 长度 ，打包时用了htons【本机序转网络序】         //要发送多少数据（包头加包体）
            //开始不把socket写事件通知加入到epoll,当我需要写数据的时候，直接调用write/send发送数据；
            //如果返回了EAGIN【发送缓冲区满了，需要等待可写事件才能继续往缓冲区里写数据】
            //此时，我再把写事件通知加入到epoll，
            //此时，就变成了在epoll驱动下写数据，全部数据发送完毕后，再把写事件通知从epoll中干掉；
            //优点：数据不多的时候，可以避免epoll的写事件的增加/删除，提高了程序的执行效率；                         
            ngx_log_stderr(errno, "即将发送数据，发送数据大小为%ud。", p_Conn->isendlen);
            ssize_t sendsize = sendproc(p_Conn, p_Conn->psendbuf, p_Conn->isendlen); //注意参数

            if (sendsize > 0)
            {
                if (sendsize == p_Conn->isendlen) //成功发送出去了数据，一下就发送出去这很顺利
                {
                    p_Conn->psendMemPointer.reset(); // unique_ptr 释放内存并置空
                    ngx_log_stderr(0, "CSocekt::ServerSendQueueThread()中数据发送完毕！");
                    printf("\n");
                    printf("\n");
                    printf("\n");
                }
                else
                {
                    p_Conn->psendbuf = p_Conn->psendbuf + sendsize;
                    p_Conn->isendlen = p_Conn->isendlen - sendsize;
                    ++p_Conn->iThrowsendCount;             //标记发送缓冲区满了，需要通过epoll事件来驱动消息的继续发送
                    ngx_epoll_oper_event(p_Conn->fd, EPOLL_CTL_MOD, EPOLLOUT, 0, p_Conn);
                    ngx_log_stderr(errno, "CSocekt::ServerSendQueueThread()中数据没发送完毕【发送缓冲区满】，要发送%d，实际发送了%d!",
                        p_Conn->isendlen + sendsize, sendsize);
                }
            }
            else
            {
                if (sendsize == -1)  // 发送缓冲区满了
                {
                    ++p_Conn->iThrowsendCount;
                    ngx_epoll_oper_event(p_Conn->fd, EPOLL_CTL_MOD, EPOLLOUT, 0, p_Conn);
                }
                else
                {
                    ngx_log_stderr(errno, "CSocekt::ServerSendQueueThread()中sendproc()返回0，直接释放内存！(对方关闭连接)");
                    p_Conn->psendMemPointer.reset(); // unique_ptr 释放内存并置空
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        ngx_log_stderr(0, "ServerSendQueueLoop caught an exception: %s", e.what());
    }
}
