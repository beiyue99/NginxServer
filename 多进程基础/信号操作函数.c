



//信号


信号产生函数：
int kill(pid_t pid, int sig);        给指定进程发送指定信号



pid有四种情况：
pid > 0: 将信号传给进程为pid的进程           pid = 0：将信号传递给当前进程所在进程组中的所有进程
pid = -1：将信号传送给系统内所有的进程       pid < -1：将信号传给指定进程组中的所有进程，这个进程组等于pid的绝对值











信号一：睡眠10秒，信号二：睡眠三秒
假如程序可以捕获信号一和信号二，如果先收到信号一，正睡眠10秒，然后忽然来个信号二，
那么会跳转处理信号二，此时信号一被打断，信号二处理完之后，继续进入信号一的睡眠, 共13秒



#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void handleSignal(int signal) {
    if (signal == SIGUSR1) {
        printf("Received Signal 1 - Sleeping for 10 seconds\n");
        printf("将要睡10秒\n");
        sleep(10);
        printf("睡了10秒\n");

    }
    else if (signal == SIGUSR2) {
        printf("Received Signal 2 during sleep\n");

        printf("将要睡3秒\n");

        sleep(3);
        printf("睡了3秒\n");

        // Handle Signal 2 during sleep
    }
}

int main() {
    // 设置信号处理函数
    signal(SIGUSR1, handleSignal);
    signal(SIGUSR2, handleSignal);

    printf("Starting main loop\n");

    while (1) {
        // 主循环
        sleep(1);
        printf("睡了1秒\n");
    }

    return 0;
}

睡了1秒
睡了1秒
Received Signal 1 - Sleeping for 10 seconds
将要睡10秒
Received Signal 2 during sleep
将要睡3秒
睡了3秒
睡了10秒
睡了1秒
睡了1秒
如果收到信号一睡眠期间，多次收到信号1，则视为只收到一次信号1，当前睡眠时间过后，继续睡眠一次这个时间




信号产生且被响应叫做递达状态，信号产生但没有响应叫未决状态
信号产生后，内核把信号发给进程，进程中维护着两个信号集（未决信号集和阻塞信号集），
如果阻塞信号集的值是1，则表示不能通过，信号不能响应
未决信号集可以读，不可以设置，由内核自动设置，用户可以设置阻塞信号集







int sigemptyset(sigset_t* set)       //将集和置空
int sigfillset(sigset_t* set);       //将所有信号加入set集和
int sigdelset   ...   //删除
int sigaddset(sigset_t* set, int signo);//将signo信号加入集和
int sigismember(const sigset_t* set, int signo);//判断信号signo是否存在





7 int main()
8 {
9     int i = 0;
10     //信号集集和
11     sigset_t set;
12     //清空集合
13     sigemptyset(&set);
14     for (i = 1; i < 32; i++)         //共有31个信号
15     {
16         if (sigismember(&set, i))
17         {
18             printf("1");
19         }
20         else
21         {
22             printf("0");
23         }
24     }
25     return 0;
26 }







//signal函数 
//sighandler_t  是一个函数指针，返回值为void,参数类型为int

sighandler_t signal(int signum, sighandler_t handler);

handler : 有三种取值
1.SIG_IGN : 忽略该信号
2.SIG_DFL : 执行系统默认操作
3.信号处理函数名 : 自定义信号处理函数

返回值： 成功的话，第一次返回NULL, 下一次返回此信号上一次注册的信号处理函数的地址。
如果需要使用此返回值，必须在前面先声明此函数指针的类型
失败返回 SIG_ERR

在此函数中，用不可重入函数，可能出现问题
//该函数尽量避免使用，用sigaction函数取而代之





8 void fun1(int signum)
9 {
10     printf("捕捉到信号：%d\n", signum);
11 }
12 void fun2(int signum)
13 {
14     printf("捕捉到信号: %d\n", signum);
15 }
16 int main()
17 {
18     signal(SIGINT, fun1);
19     signal(SIGQUIT, fun2);
20     while (1)
21     {
22         sleep(1);
23     }
24     return 0;
25 }















int sigprocmask(int how, const sigset_t* set, sigset_t* oldset);
//检查或修改信号阻塞集，根据how指定的方法对进程的阻塞集进行修改，新的信号阻塞集由set指定，原先的由oldset保存
//参数 set   要操作的信号集地址      若set为NULL，函数只会把当前的信号阻塞集合保存到oldset中
成功返回0失败返回 - 1，失败时错误代码可能是EINVAL，表示how不合法




#include<sys/wait.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/time.h>

void fun1(int signum)
{
    printf("捕捉到信号%d\n", signum);
}

void fun2(int signum)
{
    printf("捕捉到信号%d\n", signum);
}
int main()
{
    int ret = -1;
    sigset_t set;
    sigset_t oldset;
    //ctrl + C
    signal(SIGINT, fun1);
    //ctrl + \ 
    signal(SIGQUIT, fun2);
    printf("按下任意键，阻塞信号2\n");
    getchar();
    sigemptyset(&oldset);
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    ret = sigprocmask(SIG_BLOCK, &set, &oldset);//阻塞信号2
    if (ret == -1)
    {
        perror("sigprocmask");
        return 1;
    }
    printf("设置阻塞编号为2的信号成功...\n");
    printf("按下任意键解除编号为2的信号的阻塞..\n");
    getchar();
    ret = sigprocmask(SIG_SETMASK, &oldset, NULL);   //替换为原来，即为非阻塞
    if (ret == -1)
    {
        perror("sigprocmask");
        return 1;
    }
    printf("按下任意键退出..\n");
    getchar();
    return 0;
}








sigaction 函数

int sigaction(int signum, const struct sigaction* act, struct sigaction* oldact);



struct sigaction
{
    void(*sa_handler)(int);//旧的信号处理函数指针

    void(*sa_sigaction)(int, siginfo_t*, void*);//新的信号处理函数指针

    sigset_t sa_mask;   //信号阻塞集

    int sa_flags;    //信号处理的方式

    void(*sa_restorer)(void);//已弃用
};




//1)
//sa_handler、sa_sigaction:信号处理函数指针, 和signal()里的函数指针用法一样, 应根据情况给
//sa_sigaction、sa_handler两者之一赋值, 其取值如下 :
//    a)SIG_IGN : 忽略该信号
//    b)SIG_DFL:执行系统默认动作
//    c)处理函数名:自定义信号处理函数
//
//
//    2)
//    sa_mask:信号阻塞集, 在信号处理函数执行过程中, 临时屏蔽指定的信号。
//
//
//    3)
//    sa_flgs:用于指定信号处理的行为, 通常设置为0, 表使用默认属性。
// 它可以是一下值的“按位或"组合:
//    SA NOCLDSTOP : 使父进程在它的子进程暂停或继续运行时不会收到SIGCHLD信号。
//    SA NOCLDWAIT : 使父进程在它的子进程退出时不会收到SIGCHLD信号, 这时子进程如果退出也不会成
//    为僵尸进程。
//    SA NODEFER : 使对信号的屏蔽无效, 即在信号处理函数执行期间仍能发出这个信号。
//    SA_RESETHAND : 信号处理之后重新设置为默认的处理方式。
//    SA_SIGINFO : 使用sa_sigaction成员而不是sa_handler作为信号处理函数。





void fun(int signo)
{
    printf("捕捉到信号%d\n", signo);
}
//演示sigaction函数使用
int main(void)
{
    int ret = -1;
    struct sigaction act;
    //使用旧的信号处理函数指针
    act.sa_handler = fun;
    //标志为默认默认使用旧的信号处理函数指针
    act.sa_flags = 0;
    //信号注册
    ret = sigaction(SIGINT, &act, NULL);
    printf("屏蔽信号2成功！\n");
    while (1)
    {
        sleep(1);
        printf("睡眠了1秒\n");
    }
    return 0;
}







//int sigsuspend(const sigset_t* mask);
//该函数的目的是将当前进程的信号屏蔽集替换为由 mask 参数指定的信号集，然后挂起进程等待信号的到来。
//一旦收到信号，进程会恢复到原先的信号屏蔽集，并继续执行。
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include<unistd.h>
void signal_handler(int signo) {
    printf("Received signal %d\n", signo);
    whi1e(1)
    {
        sleep(1);
        printf("休息一秒\n");
    }
}

int main() {
    // 设置信号处理函数
    signal(SIGINT, signal_handler);

    // 构造一个信号集，只包含 SIGINT 信号
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);

    // 保存当前的信号屏蔽集
    sigset_t prev_mask;
    sigprocmask(SIG_BLOCK, &mask, &prev_mask);

    printf("Waiting for SIGINT...\n");

    // 挂起进程等待 SIGINT 信号
    sigsuspend(&prev_mask);
    return 0;
}



