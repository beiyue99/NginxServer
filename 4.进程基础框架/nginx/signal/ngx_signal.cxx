//和信号有关的函数放这里
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>    //信号相关头文件 
#include <errno.h>     //errno

#include "ngx_macro.h"
#include "ngx_func.h" 

//一个信号有关的结构 ngx_signal_t
typedef struct 
{
    int           signo;      
    const  char   *signame;    //信号对应的中文名字 ，比如SIGHUP 

    //信号处理函数,这个函数由我们自己来提供，但是它的参数和返回值是固定的【操作系统就这样要求】
    void  (*handler)(int signo, siginfo_t *siginfo, void *ucontext); //函数指针,   siginfo_t:系统定义的结构
} ngx_signal_t;

//声明一个信号处理函数
static void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext); //static表示该函数只在当前文件内可见

ngx_signal_t  signals[] = {
    // signo      signame             handler
    { SIGHUP,    "SIGHUP",           ngx_signal_handler },        //终端断开信号，对于守护进程常用于reload重载配置文件通知--标识1
    { SIGINT,    "SIGINT",           ngx_signal_handler },        //标识2   
	{ SIGTERM,   "SIGTERM",          ngx_signal_handler },        //标识15
    { SIGCHLD,   "SIGCHLD",          ngx_signal_handler },        //子进程退出时，父进程会收到这个信号--标识17
    { SIGQUIT,   "SIGQUIT",          ngx_signal_handler },        //标识3
    { SIGIO,     "SIGIO",            ngx_signal_handler },        //指示一个异步I/O事件【通用异步I/O信号】
    { SIGSYS,    "SIGSYS, SIG_IGN",  NULL               },        //我们想忽略这个信号，SIGSYS表示收到了一个无效系统调用，
                                                                  //如果我们不忽略，进程会被操作系统杀死，--标识31
                                                                  //所以我们把handler设置为NULL，代表 我要求忽略这个信号，
                                                                  //请求操作系统不要执行缺省的该信号处理动作（杀掉我）
    //...日后根据需要再继续增加
    { 0,         NULL,               NULL               }         //信号对应的数字至少是1，所以可以用0作为一个特殊标记
};

int ngx_init_signals()
{
    ngx_signal_t      *sig;  //指向自定义结构数组的指针 
    struct sigaction   sa;   //sigaction：系统定义的跟信号有关的一个结构

    for (sig = signals; sig->signo != 0; sig++)  //将signo ==0作为一个标记，因为信号的编号都不为0；
    {        
        //我们注意，现在要把一堆信息往变量sa对应的结构里弄 ......
        memset(&sa,0,sizeof(struct sigaction));
        if (sig->handler)  //如果信号处理函数不为空，这当然表示我要定义自己的信号处理函数
        {
            sa.sa_sigaction = sig->handler;  
            sa.sa_flags = SA_SIGINFO;        
        }
        else
        {
            sa.sa_handler = SIG_IGN;
            //sa_handler:这个标记SIG_IGN给到sa_handler成员，表示忽略信号的处理程序，
            // 否则操作系统的缺省信号处理程序很可能把这个进程杀掉；
            //其实sa_handler和sa_sigaction都是一个函数指针用来表示信号处理程序。
        } 
        sigemptyset(&sa.sa_mask);   
        if (sigaction(sig->signo, &sa, NULL) == -1) 
        {   
            ngx_log_error_core(NGX_LOG_EMERG,errno,"sigaction(%s) failed",sig->signame); //显示到日志文件中去的 
            return -1; //有失败就直接返回
        }	
        else
        {            
            //ngx_log_error_core(NGX_LOG_EMERG,errno,"sigaction(%s) succed!",sig->signame);     //成功不用写日志 
            ngx_log_stderr(0,"sigaction(%s) succed!",sig->signame); //直接往屏幕上打印看看 ，不需要时可以去掉
        }
    } //end for
    return 0; //成功    
}

//信号处理函数
static void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext)
{
    printf("来信号了\n");
}
