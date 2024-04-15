
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>   //信号相关头文件 
#include <errno.h>    //errno
#include <unistd.h>

#include "ngx_func.h"
#include "ngx_macro.h"
#include "ngx_c_conf.h"

//函数声明
static void ngx_start_worker_processes(int threadnums);
static int ngx_spawn_process(int threadnums,const char *pprocname);
static void ngx_worker_process_cycle(int inum,const char *pprocname);
static void ngx_worker_process_init(int inum);

//变量声明
static u_char  master_process[] = "master process";







//调用ngx_start_worker_processes创建worker子进程
void ngx_master_process_cycle()
{    
    sigset_t set;        //信号集
    sigemptyset(&set);   //清空信号集

    //建议fork()子进程时用这种写法，防止信号的干扰；
    sigaddset(&set, SIGCHLD);     //子进程状态改变
    sigaddset(&set, SIGALRM);     //定时器超时
    sigaddset(&set, SIGIO);       //异步I/O
    sigaddset(&set, SIGINT);      //终端中断符
    sigaddset(&set, SIGHUP);      //连接断开
    sigaddset(&set, SIGUSR1);     //用户定义信号
    sigaddset(&set, SIGUSR2);     //用户定义信号
    sigaddset(&set, SIGWINCH);    //终端窗口大小改变
    sigaddset(&set, SIGTERM);     //终止
    sigaddset(&set, SIGQUIT);     //终端退出符
    
    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) 
    {        
        ngx_log_error_core(NGX_LOG_ALERT,errno,"ngx_master_process_cycle()中sigprocmask()失败!");
    }

    size_t size;
    int    i;
    size = sizeof(master_process);  //注意我这里用的是sizeof，所以字符串末尾的\0是被计算进来了的
    size += g_argvneedmem;          //argv参数长度加进来    
    if(size < 1000) //长度小于这个，我才设置标题
    {
        char title[1000] = {0};
        strcpy(title,(const char *)master_process); //"master process"
        strcat(title," ");  //跟一个空格分开一些，清晰    //"master process "
        for (i = 0; i < g_os_argc; i++)         //"master process ./nginx"
        {
            strcat(title,g_os_argv[i]);
        }//end for
        ngx_setproctitle(title); //设置标题
        ngx_log_error_core(NGX_LOG_NOTICE,0,"%s %P 【master进程】启动并开始运行......!",title,ngx_pid); 
    }    
    //从配置文件中读取要创建的worker进程数量
    CConfig *p_config = CConfig::GetInstance(); //单例类
    int workprocess = p_config->GetIntDefault("WorkerProcesses",1); //从配置文件中得到要创建的worker进程数量
    ngx_start_worker_processes(workprocess);  //这里要创建worker子进程

    sigemptyset(&set); //信号屏蔽字为空，表示不屏蔽任何信号
    for ( ;; ) 
    {

        sigsuspend(&set); 
        printf("master进程休息1秒\n");      
        sleep(1); //休息1秒        
    }
    return;
}



//调用ngx_spawn_process创建子进程
static void ngx_start_worker_processes(int threadnums)
{
    int i;
    for (i = 0; i < threadnums; i++)  //master进程在走这个循环，来创建若干个子进程
    {
        ngx_spawn_process(i,"worker process");
    } 
    return;
}


//创建子进程，所有子进程执行ngx_worker_process_cycle，该函数循环调用ngx_process_events_and_timers处理事件
static int ngx_spawn_process(int inum,const char *pprocname)
{
    pid_t  pid;

    pid = fork(); //fork()系统调用产生子进程
    switch (pid)  //pid判断父子进程，分支处理
    {  
    case -1: //产生子进程失败
        ngx_log_error_core(NGX_LOG_ALERT,errno,"ngx_spawn_process()fork()产生子进程num=%d,procname=\"%s\"失败!",inum,pprocname);
        return -1;

    case 0:  //子进程分支
        ngx_parent = ngx_pid;              //因为是子进程了，所有原来的pid变成了父pid
        ngx_pid = getpid();                //重新获取pid,即本子进程的pid
        ngx_worker_process_cycle(inum,pprocname);    //我希望所有worker子进程，在这个函数里不断循环着不出来，也就是说，子进程流程不往下边走;
        break;

    default: //这个应该是父进程分支，直接break;，流程往switch之后走            
        break;
    }

    return pid;
}



//循环调用ngx_process_events_and_timers处理事件
static void ngx_worker_process_cycle(int inum,const char *pprocname) 
{
    //设置一下变量
    ngx_process = NGX_PROCESS_WORKER;  //设置进程的类型，是worker进程

    ngx_worker_process_init(inum);
    ngx_setproctitle(pprocname); //设置标题   
    ngx_log_error_core(NGX_LOG_NOTICE,0,"%s %P 【worker进程】启动并开始运行......!",pprocname,ngx_pid);


    //setvbuf(stdout,NULL,_IONBF,0); //这个函数. 直接将printf缓冲区禁止， printf就直接输出了。
    for(;;)
    {
        ngx_process_events_and_timers(); //处理网络事件和定时器事件

    }

    g_threadpool.StopAll();      //考虑在这里停止线程池；
    g_socket.Shutdown_subproc(); //socket需要释放的东西考虑释放；
    return;
}



//子进程工作时调用本函数进行一些初始化工作,被ngx_worker_process_cycle函数调用
//创建线程池，初始化锁和信号量，创建三大线程：发数据线程、回收连接线程、处理不发心跳包用户线程
//创建epoll，创建连接，放入连接链表。在epoll添加监听的套间字事件
static void ngx_worker_process_init(int inum)
{
    sigset_t  set;      //信号集

    sigemptyset(&set);  //清空信号集
    if (sigprocmask(SIG_SETMASK, &set, NULL) == -1) 
    {
        ngx_log_error_core(NGX_LOG_ALERT,errno,"ngx_worker_process_init()中sigprocmask()失败!");
    }

    //线程池代码，率先创建，至少要比和socket相关的内容优先
    CConfig *p_config = CConfig::GetInstance();
    int tmpthreadnums = p_config->GetIntDefault("ProcMsgRecvWorkThreadCount",5); //处理接收消息的线程池中线程数量
    if(g_threadpool.Create(tmpthreadnums) == false)  //创建线程池中线程
    {
        exit(-2);
    }
    sleep(1); //再休息1秒；

    //初始化锁和信号量，创建三大线程：发数据线程、回收连接线程、处理不发心跳包用户线程
    if(g_socket.Initialize_subproc() == false) //初始化子进程需要具备的一些多线程能力相关的信息
    {
        exit(-2);
    }
    g_socket.ngx_epoll_init();         
    return;
}
