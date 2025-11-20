
#ifndef __NGX_SOCKET_H__
#define __NGX_SOCKET_H__

#include <vector>       //vector
#include <list>         //list
#include <sys/epoll.h>  //epoll
#include <sys/socket.h>
#include <pthread.h>    //多线程
#include <semaphore.h>  //信号量 
#include <atomic>       //c++11里的原子操作
#include <map>          //multimap
#include<memory>
#include<mutex>
#include<condition_variable>
#include<cstdint>
#include "ngx_c_threadpool.h"
#include "ngx_comm.h"
#include "ngx_c_memory.h"  

//一些宏定义放在这里-----------------------------------------------------------
#define NGX_LISTEN_BACKLOG  511    //已完成连接队列
#define NGX_MAX_EVENTS      512    //epoll_wait一次最多接收这么多个事件

// 前向声明
struct ngx_connection_s;
struct ngx_listening_s;
class  CSocekt;

// 连接的所有权与引用类型
using ngx_connection_sp = std::shared_ptr<ngx_connection_s>; // 拥有者使用
using ngx_connection_wp = std::weak_ptr<ngx_connection_s>;   // 非拥有者使用
using ngx_listening_sp = std::shared_ptr<ngx_listening_s>;
using ngx_listening_wp = std::weak_ptr<ngx_listening_s>;

using BufferPtr = std::unique_ptr<char[]>;


// 成员函数事件处理指针：改为接收 shared_ptr
using ngx_event_handler_pt = void (CSocekt::*)(ngx_connection_sp c);

//--------------------------------------------
struct ngx_listening_s  //和监听端口有关的结构
{
	int                       port = 0;        //监听的端口号
	int                       fd = -1;          //套接字句柄socket
	ngx_connection_wp         connection;    //连接池中的一个连接
};

struct ngx_connection_s: public std::enable_shared_from_this<ngx_connection_s>
{		
	ngx_connection_s() : epoll_weak_ptr(nullptr) {}
	// 析构函数中清理
	~ngx_connection_s() {
		if (epoll_weak_ptr) {
			delete static_cast<std::weak_ptr<ngx_connection_s>*>(epoll_weak_ptr);
			epoll_weak_ptr = nullptr;
		}
	}
	void GetOneToUse();                                      //分配出去的时候初始化一些内容
	void PutOneToFree();                                     //回收回来的时候做一些事情


	int                       fd = -1;                            //套接字句柄socket
	ngx_listening_wp          listening;

	uint64_t                  iCurrsequence = 0;           
	struct sockaddr           s_sockaddr {};                    //保存对方地址信息用的

	ngx_event_handler_pt      rhandler = nullptr;                       //读事件的相关处理方法
	ngx_event_handler_pt      whandler = nullptr;                        //写事件的相关处理方法

	//和epoll事件有关
	uint32_t                  events = 0;                         //和epoll事件有关  
	
	//和收包有关
	unsigned char             curStat = 0;                        //当前收包的状态
	char                      dataHeadInfo[_DATA_BUFSIZE_]{};   //用于保存收到的数据的包头信息			
	char                      *precvbuf = nullptr;               //接收数据的缓冲区的头指针
	unsigned int              irecvlen = 0;                //要收到多少数据，由这个变量指定
	BufferPtr			     precvMemPointer;         // 拥有接收缓冲（自动释放）

	std::mutex        logicPorcMutex;                     // 逻辑处理互斥

	//和发包有关
	std::atomic<int>          iThrowsendCount{ 0 };        
	//发送消息，如果发送缓冲区满了，则需要通过epoll事件来驱动消息的继续发送，所以如果发送缓冲区满，则用这个变量标记
	//char                      *psendMemPointer;        //整个数据的头指针，其实是 消息头 + 包头 + 包体
	BufferPtr				  psendMemPointer;

	char                      *psendbuf = nullptr;               //发送数据的缓冲区的头指针，开始 其实是包头+包体
	unsigned int              isendlen = 0;                //要发送多少数据

	//和回收有关
	time_t                    inRecyTime = 0;              //入到资源回收站里去的时间

	//和心跳包有关
	time_t                    lastPingTime = 0;            //上次ping的时间【上次发送心跳包的时间】

	//和网络安全有关	
	uint64_t                  FloodkickLastTime = 0;       //Flood攻击上次收到包的时间
	int                       FloodAttackCount = 0;        //Flood攻击在该时间内收到包的次数统计
	
	//lpngx_connection_t        next;                    //这是个指针，指向下一个本类型对象，用于把空闲的连接池对象串起来构成一个单向链表，方便取用
	// 空闲链表游标（仅连接池内部使用；不拥有）
	//ngx_connection_s* next_free = nullptr;
	void* epoll_weak_ptr = nullptr;  // ✅ 用于保存 epoll_event.data.ptr
};

//消息头，引入的目的是当收到数据包时，额外记录一些内容以备将来使用
struct STRUC_MSG_HEADER
{
	ngx_connection_wp  pConn;         //记录对应的链接，注意这是个指针
	uint64_t           iCurrsequence = 0; //收到数据包时记录对应连接的序号，将来能用于比较是否连接已经作废用
};   

// 为“智能指针自动释放”方便，给消息头定义 unique_ptr 别名
using LPSTRUC_MSG_HEADER = std::unique_ptr<STRUC_MSG_HEADER>;


//------------------------------------
//socket相关类
class CSocekt
{
public:
	CSocekt();                                                            //构造函数
	virtual ~CSocekt();                                                   //释放函数
	virtual bool Initialize();                                            //初始化函数[父进程中执行]
	virtual bool Initialize_subproc();                                    //初始化函数[子进程中执行]
	virtual void Shutdown_subproc();                                      //关闭退出函数[子进程中执行]

public:
	virtual void threadRecvProcFunc(char *pMsgBuf);                       //处理客户端请求，虚函数，因为将来可以考虑自己来写子类继承本类
	virtual void procPingTimeOutChecking(LPSTRUC_MSG_HEADER tmpmsg,time_t cur_time);  //心跳包检测时间到，该去检测心跳包是否超时的事宜，本函数只是把内存释放，子类应该重新事先该函数以实现具体的判断动作

public:	
	int  ngx_epoll_init();                                                //epoll功能初始化	
	int  ngx_epoll_process_events(int timer);                             //epoll等待接收和处理事件

	int ngx_epoll_oper_event(int fd,uint32_t eventtype,uint32_t flag,int bcaction, ngx_connection_sp pConn);
	                                                                      //epoll操作事件
	
protected:
	//数据发送相关
	void msgSend(BufferPtr psendbuf);
	void zdClosesocketProc(ngx_connection_sp p_Conn);                    //主动关闭一个连接时的要做些善后的处理函数	
	
private:	
	void ReadConf();                                                      //专门用于读各种配置项	
	bool ngx_open_listening_sockets();                                    //监听必须的端口【支持多个端口】
	void ngx_close_listening_sockets();                                   //关闭监听套接字
	bool setnonblocking(int sockfd);                                      //设置非阻塞套接字	

	//一些业务处理函数handler
	void ngx_event_accept(ngx_connection_sp oldc);                       //建立新连接
	void ngx_read_request_handler(ngx_connection_sp pConn);              //设置数据来时的读处理函数
	void ngx_write_request_handler(ngx_connection_sp pConn);             //设置数据发送时的写处理函数
	void ngx_close_connection(ngx_connection_sp pConn);                  //通用连接关闭函数，资源用这个函数释放【因为这里涉及到好几个要释放的资源，所以写成函数】

	ssize_t recvproc(ngx_connection_sp pConn,char *buff,ssize_t buflen); //接收从客户端来的数据专用函数
	void ngx_wait_request_handler_proc_p1(ngx_connection_sp pConn,bool &isflood);
	                                                                      //包头收完整后的处理，我们称为包处理阶段1：写成函数，方便复用      
	void ngx_wait_request_handler_proc_plast(ngx_connection_sp pConn,bool &isflood);
	                                                                      //收到一个完整包后的处理，放到一个函数中，方便调用	
	void clearMsgSendQueue();                                             //处理发送消息队列  

	ssize_t sendproc(ngx_connection_sp c,char *buff,ssize_t size);       //将数据发送到客户端 

	//获取对端信息相关                                              
	size_t ngx_sock_ntop(struct sockaddr *sa,int port,u_char *text,size_t len);  //根据参数1给定的信息，获取地址端口字符串，返回这个字符串的长度

	//连接池 或 连接 相关
	void initconnection();                                                //初始化连接池
	void clearconnection();                                               //回收连接池
	ngx_connection_sp ngx_get_connection(int isock);                     //从连接池中获取一个空闲连接
	void ngx_free_connection(ngx_connection_sp pConn);                   //归还参数pConn所代表的连接到到连接池中	
	void inRecyConnectQueue(ngx_connection_sp pConn);                    //将要回收的连接放到一个队列中来
	
	//和时间相关的函数
	void    AddToTimerQueue(ngx_connection_sp pConn);                    //设置踢出时钟(向map表中增加内容)
	time_t  GetEarliestTime();                                            //从multimap中取得最早的时间返回去
	LPSTRUC_MSG_HEADER RemoveFirstTimer();                                //从m_timeQueuemap移除最早的时间，并把最早这个时间所在的项的值所对应的指针 返回，调用者负责互斥，所以本函数不用互斥，
	LPSTRUC_MSG_HEADER GetOverTimeTimer(time_t cur_time);                  //根据给的当前时间，从m_timeQueuemap找到比这个时间更老（更早）的节点【1个】返回去，这些节点都是时间超过了，要处理的节点      
	void DeleteFromTimerQueue(ngx_connection_sp pConn);                  //把指定用户tcp连接从timer表中抠出去
	void clearAllFromTimerQueue();                                        //清理时间队列中所有内容

	//和网络安全有关
	bool TestFlood(ngx_connection_sp pConn);                             //测试是否flood攻击成立，成立则返回true，否则返回false

	
	//线程相关函数
	void ServerSendQueueLoop();      //专门用来发送数据的线程
	void ServerRecyConnectionLoop();        //专门用来回收连接的线程
	void ServerTimerQueueMonitorLoop();        //时间队列监视线程，处理到期不发心跳包的用户踢出的线程
	
	
protected:
	//一些和网络通讯有关的成员变量
	size_t                         m_iLenPkgHeader = 8;                       //sizeof(COMM_PKG_HEADER);		
	size_t                         m_iLenMsgHeader = 8;                       //sizeof(STRUC_MSG_HEADER);

	//时间相关
	int                            m_ifTimeOutKick = 0;                       //当时间到达Sock_MaxWaitTime指定的时间时，直接把客户端踢出去，只有当Sock_WaitTimeEnable = 1时，本项才有用 
	int                            m_iWaitTime = 0;                           //多少秒检测一次是否 心跳超时，只有当Sock_WaitTimeEnable = 1时，本项才有用	
	
private:
	struct ThreadItem
	{
		std::thread   _Handle;                        //线程句柄
		bool ifrunning = false;                      //标记是否正式启动起来，启动起来后，才允许调用StopAll()来释放
		CSocekt* _owner = nullptr;
		ThreadItem(CSocekt* owner) {
			_owner = owner;
		}
	};


	int                            m_worker_connections = 0;                  //epoll连接的最大项数
	int                            m_ListenPortCount = 0;                     //所监听的端口数量
	int                            m_epollhandle = 0;                         //epoll_create返回的句柄

	//和连接池有关的
	std::list<ngx_connection_sp>  m_connectionList;                      //连接列表【连接池】
	std::list<ngx_connection_sp>  m_freeconnectionList;                  //空闲连接列表【这里边装的全是空闲的连接】
	std::atomic<int>               m_total_connection_n{ 0 };                  //连接池总连接数
	std::atomic<int>               m_free_connection_n{ 0 };                   //连接池空闲连接数
	std::mutex                    m_connectionMutex;
	std::mutex                    m_recyconnqueueMutex;
	std::list<ngx_connection_sp>  m_recyconnectionList;                  //将要释放的连接放这里
	std::atomic<int>               m_totol_recyconnection_n{ 0 };              //待释放连接队列大小
	int                            m_RecyConnectionWaitTime = 0;              //等待这么些秒后才回收连接


	
	std::vector<ngx_listening_sp> m_ListenSocketList;                    //监听套接字队列
	struct epoll_event             m_events[NGX_MAX_EVENTS];              //用于在epoll_wait()中承载返回的所发生的事件

	//消息队列
	std::list<BufferPtr>		   m_MsgSendQueue;
	std::atomic<int>               m_iSendMsgQueueCount{ 0 };                  //发消息队列大小
	//多线程相关
	std::vector<std::unique_ptr<ThreadItem>>      m_threadVector;                        //线程 容器，容器里就是各个线程了 	
	// 现代锁与条件变量
	std::mutex                    m_sendMessageQueueMutex;
	std::condition_variable       m_sendMessageQueueCv;
	//时间相关
	int                            m_ifkickTimeCount = 0;                     //是否开启踢人时钟，1：开启   0：不开启		
	std::mutex                    m_timequeueMutex;
	std::multimap<time_t, LPSTRUC_MSG_HEADER>   m_timerQueuemap;          //时间队列	
	size_t                         m_cur_size_{ 0 };                           //时间队列的尺寸
	time_t                         m_timer_value_{ 0 };                        //当前计时队列头部时间值
	//在线用户相关
	std::atomic<int>               m_onlineUserCount{ 0 };                     //当前在线用户数统计
	//网络安全相关
	int                            m_floodAkEnable = 0;                       //Flood攻击检测是否开启,1：开启   0：不开启
	unsigned int                   m_floodTimeInterval = 0;                   //表示每次收到数据包的时间间隔是100(毫秒)
	int                            m_floodKickCount = 0;                      //累积多少次踢出此人

	// 停止标志（替代全局 g_stopEvent，建议放到类里）
	std::atomic<bool> m_stop{ false };
};

#endif
