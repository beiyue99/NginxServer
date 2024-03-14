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

//构造函数
CSocekt::CSocekt()
{
    //配置相关
    m_worker_connections = 1;    //epoll连接最大项数
    m_ListenPortCount = 1;       //监听一个端口
    //epoll相关
    m_epollhandle = -1;          //epoll返回的句柄
    m_pconnections = NULL;       //连接池【连接数组】先给空
    m_pfree_connections = NULL;  //连接池中空闲的连接链 
    return;	
}
//释放函数
CSocekt::~CSocekt()
{
    std::vector<lpngx_listening_t>::iterator pos;	
	for(pos = m_ListenSocketList.begin(); pos != m_ListenSocketList.end(); ++pos) //vector
	{		
		delete (*pos); //一定要把指针指向的内存干掉，不然内存泄漏
	}//end for
	m_ListenSocketList.clear(); 
    if(m_pconnections != NULL)//释放连接池
        delete [] m_pconnections;
    return;
}

//初始化函数【fork()子进程之前干这个事】
bool CSocekt::Initialize()
{
    ReadConf();  //读配置项
    bool reco = ngx_open_listening_sockets();  //打开监听端口
    return reco;
}

//专门用于读各种配置项
void CSocekt::ReadConf()
{
    CConfig *p_config = CConfig::GetInstance();
    m_worker_connections = p_config->GetIntDefault("worker_connections",m_worker_connections); //epoll连接的最大项数
    m_ListenPortCount    = p_config->GetIntDefault("ListenPortCount",m_ListenPortCount);       //取得要监听的端口数量
    return;
}

//在创建worker进程之前就要执行这个函数；
bool CSocekt::ngx_open_listening_sockets()
{    
    int                isock;                //socket
    struct sockaddr_in serv_addr;            //服务器的地址结构体
    int                iport;                //端口
    char               strinfo[100];         //临时字符串 
   
    //初始化相关
    memset(&serv_addr,0,sizeof(serv_addr));  //先初始化一下
    serv_addr.sin_family = AF_INET;                //选择协议族为IPV4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //（服务器可能不止一个网卡）多个本地ip地址都进行绑定端口号，进行侦听。

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
        int reuseaddr = 1;  //1:打开对应的设置项
        if(setsockopt(isock,SOL_SOCKET, SO_REUSEADDR,(const void *) &reuseaddr, sizeof(reuseaddr)) == -1)
        {
            ngx_log_stderr(errno,"CSocekt::Initialize()中setsockopt(SO_REUSEADDR)失败,i=%d.",i);
            close(isock); //无需理会是否正常执行了                                                  
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
        //可以，放到列表里来
        lpngx_listening_t p_listensocketitem = new ngx_listening_t;
        memset(p_listensocketitem,0,sizeof(ngx_listening_t));     
        p_listensocketitem->port = iport;                         
        p_listensocketitem->fd   = isock;                         
        ngx_log_error_core(NGX_LOG_INFO,0,"监听%d端口成功!",iport); //显示一些信息到日志中
        m_ListenSocketList.push_back(p_listensocketitem);          //加入到队列中
    }  
    if(m_ListenSocketList.size() <= 0)  //不可能一个端口都不监听吧
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

//关闭socket，什么时候用，我们现在先不确定，先把这个函数预备在这里
void CSocekt::ngx_close_listening_sockets()
{
    for(int i = 0; i < m_ListenPortCount; i++) //要关闭这么多个监听端口
    {  
        close(m_ListenSocketList[i]->fd);
        ngx_log_error_core(NGX_LOG_INFO,0,"关闭监听端口%d!",m_ListenSocketList[i]->port); //显示一些信息到日志中
    }
    return;
}

//(1)epoll功能初始化，子进程中进行 ，本函数被ngx_worker_process_init()所调用
int CSocekt::ngx_epoll_init()
{
    m_epollhandle = epoll_create(m_worker_connections);   //直接以epoll连接的最大项数为参数，肯定是>0的； 
    if (m_epollhandle == -1) 
    {
        ngx_log_stderr(errno,"CSocekt::ngx_epoll_init()中epoll_create()失败.");
        exit(2); //这是致命问题了，直接退，资源由系统释放吧，这里不刻意释放了，比较麻烦
    }

    //(2)创建连接池【数组】、创建出来，这个东西后续用于处理所有客户端的连接
    m_connection_n = m_worker_connections;      //记录当前连接池中连接总数
    m_pconnections = new ngx_connection_t[m_connection_n]; //new不可以失败，不用判断结果，如果失败直接报异常更好一些

    int i = m_connection_n;                //连接池中连接数
    lpngx_connection_t next  = NULL;
    lpngx_connection_t c = m_pconnections; //连接池数组首地址
    do 
    {
        i--;   //注意i是数字的末尾，从最后遍历，i递减至数组首个元素
        c[i].data = next;         //设置连接对象的next指针，注意第一次循环时next = NULL;
        c[i].fd = -1;             //初始化连接，无socket和该连接池中的连接【对象】绑定
        c[i].iCurrsequence = 0;   //当前序号统一从0开始
        next = &c[i]; //next指针前移
    } while (i); //循环直至i为0，即数组首地址
    m_pfree_connections = next;            //设置空闲连接链表头指针,因为现在next指向c[0]，注意现在整个链表都是空的
    m_free_connection_n = m_connection_n;  //空闲连接链表长度，因为现在整个链表都是空的，这两个长度相等；

    std::vector<lpngx_listening_t>::iterator pos;	
	for(pos = m_ListenSocketList.begin(); pos != m_ListenSocketList.end(); ++pos)
    {
        c = ngx_get_connection((*pos)->fd); //从连接池中获取一个空闲连接对象
        if (c == NULL)
        {
            ngx_log_stderr(errno,"CSocekt::ngx_epoll_init()中ngx_get_connection()失败.");
            exit(2); //这是致命问题了，直接退，资源由系统释放吧，这里不刻意释放了，比较麻烦
        }
        c->listening = (*pos);   //连接对象 和监听对象关联，方便通过连接对象找监听对象
        (*pos)->connection = c;  //监听对象 和连接对象关联，方便通过监听对象找连接对象
        c->rhandler = &CSocekt::ngx_event_accept;

        //把监听用的套间字上树
        if(ngx_epoll_add_event((*pos)->fd,       //socekt句柄
                                1,0,             //读，写
                                0,               //其他补充标记
                                EPOLL_CTL_ADD,   //事件类型【增加，还有删除/修改】
                                c                //连接池中的连接 
                                ) == -1) 
        {
            exit(2); //有问题，直接退出，日志 已经写过了
        }
    } //end for 
    return 1;
}




int CSocekt::ngx_epoll_add_event(int fd,
                                int readevent,int writeevent,
                                uint32_t otherflag, 
                                uint32_t eventtype, 
                                lpngx_connection_t c
                                )
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    if (readevent == 1)
    {
        ev.events = EPOLLIN | EPOLLRDHUP;
    }
    else
    {
        //.....
    }
    
    if(otherflag != 0)
    {
        ev.events |= otherflag; 
    }

    ev.data.ptr = (void *) ((uintptr_t)c) ;   
    if(epoll_ctl(m_epollhandle,eventtype,fd,&ev) == -1)
    {
        ngx_log_stderr(errno,"CSocekt::ngx_epoll_add_event()中epoll_ctl(%d,%d,%d,%u,%u)失败.",
            fd,readevent,writeevent,otherflag,eventtype);
        return -1;
    }
    return 1;
}

//本函数被ngx_process_events_and_timers()调用，而ngx_process_events_and_timers()是在子进程的死循环中被反复调用
int CSocekt::ngx_epoll_process_events(int timer) 
{   
    //返回值：有错误发生返回-1，错误在errno中，比如你发个信号过来，就返回-1，错误信息是(4: Interrupted system call)
    int events = epoll_wait(m_epollhandle,m_events,NGX_MAX_EVENTS,timer); 
    if(events == -1)
    {
 
        if(errno == EINTR) 
        {
            //信号所致，直接返回，一般认为这不是毛病，但还是打印下日志记录一下，因为一般也不会人为给worker进程发送信号
            ngx_log_error_core(NGX_LOG_INFO,errno,"CSocekt::ngx_epoll_process_events()中epoll_wait()失败!"); 
            return 1;  //正常返回
        }
        else
        {
            //这被认为应该是有问题，记录日志
            ngx_log_error_core(NGX_LOG_ALERT,errno,"CSocekt::ngx_epoll_process_events()中epoll_wait()失败!"); 
            return 0;  //非正常返回 
        }
    }

    if(events == 0) //超时，但没事件来
    {
        if(timer != -1)
        {
            //要求epoll_wait阻塞一定的时间而不是一直阻塞，这属于阻塞到时间了，则正常返回
            return 1;
        }
        //无限等待【所以不存在超时】，但却没返回任何事件，这应该不正常有问题        
        ngx_log_error_core(NGX_LOG_ALERT,0,"CSocekt::ngx_epoll_process_events()中epoll_wait()没超时却没返回任何事件!"); 
        return 0; //非正常返回 
    }
   

    //走到这里，就是属于有事件收到了
    lpngx_connection_t c;
    uint32_t           revents;
    for (int i = 0; i < events; ++i)    //遍历本次epoll_wait返回的所有事件，注意events才是返回的实际事件数量
    {
        c = (lpngx_connection_t)(m_events[i].data.ptr);
        if (c->fd == -1)
        {
            //什么时候这个c->fd会变成-1呢？关闭连接时这个fd会被设置为-1，但应该不是ngx_free_connection()函数设置的-1
            //比如我们用epoll_wait取得两个事件，处理第一个事件时，因为业务需要，
            // 我们把这个连接关闭，那我们应该会把c->fd设置为-1；
            //第二个事件，假如这第二个事件，也跟第一个事件对应的是同一个连接，那这个条件就会成立；
            // 那么这种事件，属于过期事件，不该处理
            ngx_log_error_core(NGX_LOG_DEBUG, 0, "CSocekt::ngx_epoll_process_events()中遇到了fd=-1的过期事件:%p.", c);
            continue; //这种事件就不处理即可
        }

        //能走到这里，我们认为这些事件都没过期，就正常开始处理
        revents = m_events[i].events;//取出事件类型
        if (revents & (EPOLLERR | EPOLLHUP))
            //例如对方close掉套接字，这里会感应到【换句话说：如果发生了错误或者客户端断连】
        {
            //这加上读写标记，方便后续代码处理
            revents |= EPOLLIN | EPOLLOUT;
        }

        if (revents & EPOLLIN)  //如果是读事件
        {
            //一个客户端新连入，这个会成立，
            //已连接发送数据来，这个也成立；
            (this->* (c->rhandler))(c);
            //如果新连接进入，这里执行的应该是CSocekt::ngx_event_accept(c)           
            //如果是已经连入，发送数据到这里，则这里执行的应该是 CSocekt::ngx_wait_request_handler(c)
        }

        if (revents & EPOLLOUT) //如果是写事件
        {

            ngx_log_stderr(errno, "有写事件111111111111111111111111111111.");

        }
    }
    
    return 1;
}
