#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>   
#include <errno.h>     
#include <sys/wait.h>  

#include "ngx_global.h"
#include "ngx_macro.h"
#include "ngx_func.h" 

//一个信号有关的结构 ngx_signal_t
typedef struct 
{
    int           signo;      
    const  char   *signame;    

    void  (*handler)(int signo, siginfo_t *siginfo, void *ucontext); //函数指针,   siginfo_t:系统定义的结构
} ngx_signal_t;

//声明一个信号处理函数
static void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext);
static void ngx_process_get_status(void);                               

ngx_signal_t  signals[] = {
    // signo      signame             handler
    { SIGHUP,    "SIGHUP",           ngx_signal_handler },        //终端断开信号，对于守护进程常用于reload重载配置文件通知--标识1
    { SIGINT,    "SIGINT",           ngx_signal_handler },        //标识2   
	{ SIGTERM,   "SIGTERM",          ngx_signal_handler },        //标识15
    { SIGCHLD,   "SIGCHLD",          ngx_signal_handler },        //子进程退出时，父进程会收到这个信号--标识17
    { SIGQUIT,   "SIGQUIT",          ngx_signal_handler },        //标识3
    { SIGIO,     "SIGIO",            ngx_signal_handler },        //指示一个异步I/O事件【通用异步I/O信号】
    { SIGSYS,    "SIGSYS, SIG_IGN",  NULL               },        //我们想忽略这个信号，SIGSYS表示收到了一个无效系统调用，如果我们不忽略，进程会被操作系统杀死，--标识31
    { 0,         NULL,               NULL               }         //信号对应的数字至少是1，所以可以用0作为一个特殊标记
};

//初始化信号的函数，用于注册信号处理程序
//返回值：0成功  ，-1失败
int ngx_init_signals()
{
    ngx_signal_t      *sig;  //指向自定义结构数组的指针 
    struct sigaction   sa;   //sigaction：系统定义的跟信号有关的一个结构，我们后续调用系统的sigaction()函数时要用到这个同名的结构

    for (sig = signals; sig->signo != 0; sig++)  //将signo ==0作为一个标记，因为信号的编号都不为0；
    {        
        memset(&sa,0,sizeof(struct sigaction));
        if (sig->handler)  //如果信号处理函数不为空，这当然表示我要定义自己的信号处理函数
        {
            sa.sa_sigaction = sig->handler;  
            sa.sa_flags = SA_SIGINFO;        
        }
        else
        {
            sa.sa_handler = SIG_IGN;
        }

        sigemptyset(&sa.sa_mask);  
        if (sigaction(sig->signo, &sa, NULL) == -1) 
        {   
            ngx_log_error_core(NGX_LOG_EMERG,errno,"sigaction(%s) failed",sig->signame); //显示到日志文件中去的 
            return -1; //有失败就直接返回
        }	
    } //end for
    return 0; //成功    
}

//信号处理函数
//siginfo：这个系统定义的结构中包含了信号产生原因的有关信息
static void ngx_signal_handler(int signo, siginfo_t *siginfo, void *ucontext)
{    
    ngx_signal_t    *sig;    //自定义结构
    char            *action; //一个字符串，用于记录一个动作字符串以往日志文件中写
    
    for (sig = signals; sig->signo != 0; sig++) //遍历信号数组    
    {         
        //找到对应信号，即可处理
        if (sig->signo == signo) 
        { 
            break;
        }
    } //end for

    action = (char *)"";  //目前还没有什么动作；

    if(ngx_process == NGX_PROCESS_MASTER)      //master进程，管理进程，处理的信号一般会比较多 
    {
        //master进程的往这里走
        switch (signo)
        {
        case SIGCHLD:  //一般子进程退出会收到该信号
            ngx_reap = 1;  //标记子进程状态变化
            break;

        default:
            break;
        }
    }
    else if(ngx_process == NGX_PROCESS_WORKER) //worker进程，具体干活的进程，处理的信号相对比较少
    {
        //do nothing
    }
    else
    {
    } 

    //这里记录一些日志信息
    if(siginfo && siginfo->si_pid)  //si_pid = sending process ID【发送该信号的进程id】
    {
        ngx_log_error_core(NGX_LOG_NOTICE,0,"signal %d (%s) received from %P%s", signo, sig->signame, siginfo->si_pid, action); 
    }
    else
    {
        ngx_log_error_core(NGX_LOG_NOTICE,0,"signal %d (%s) received %s",signo, sig->signame, action);
    }


    if (signo == SIGCHLD) 
    {
        ngx_process_get_status(); //获取子进程的结束状态
    } 

    return;
}

static void ngx_process_get_status(void)
{
    pid_t            pid;
    int              status;
    int              err;
    int              one=0; //抄自官方nginx，应该是标记信号正常处理过一次

    //当你杀死一个子进程时，父进程会收到这个SIGCHLD信号。
    for ( ;; ) 
    {
        //第一次waitpid返回一个> 0值，表示成功
        //第二次再循环回来，再次调用waitpid会返回一个0，表示子进程还没结束，然后这里有return来退出；
        pid = waitpid(-1, &status, WNOHANG); 
        if(pid == 0) 
        {
            return;
        }
        if(pid == -1)
        {
            err = errno;
            if(err == EINTR)           //调用被某个信号中断
            {
                continue;
            }

            if(err == ECHILD  && one)  //没有子进程
            {
                return;
            }

            if (err == ECHILD)         //没有子进程
            {
                ngx_log_error_core(NGX_LOG_INFO,err,"waitpid() failed!");
                return;
            }
            ngx_log_error_core(NGX_LOG_ALERT,err,"waitpid() failed!");
            return;
        }  
        one = 1;  //标记waitpid()返回了正常的返回值
        if(WTERMSIG(status))  //获取使子进程终止的信号编号
        {
            ngx_log_error_core(NGX_LOG_ALERT,0,"pid = %P exited on signal %d!",pid,WTERMSIG(status)); //获取使子进程终止的信号编号
        }
        else
        {
            ngx_log_error_core(NGX_LOG_NOTICE,0,"pid = %P exited with code %d!",pid,WEXITSTATUS(status)); //WEXITSTATUS()获取子进程传递给exit或者_exit参数的低八位
        }
    }
    return;
}
