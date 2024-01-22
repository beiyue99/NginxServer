
孤儿进程： 每当出现一个孤儿进程，内核就会设置孤儿的父进程为init，init会循环的回收资源，因此孤儿进程没什么危害

僵尸进程:  死去的子进程不回收变成僵尸




11.1 SIGCHLD信号产生的条件(17)
1)子进程终止时，发给父进程
2)子进程接收到SIGSTOP信号停止时，发给父进程
3)子进程处在停止态, 接受到SIGCONT后唤醒时，发给父进程

父进程接收信号，防止出现僵尸进程
#include<stdio.h>
#include<signal.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>


void fun(int signo)
{
    printf("捕捉到信号:%d\n", signo);         //17
    printf("有子进程退出.,,,\n");
}

int main(void)
{
    pid_t pid = -1;
    struct sigaction act;
    act.sa_handler = fun;
    act.sa_flags = 0;
    //注册信号处理函数
    sigaction(SIGCHLD, &act, NULL);
    //创建一个子进程
    pid = fork();
    if (-1 == pid)
    {
        perror("fork");
        return 1;
    }
    //子进程
    if (0 == pid)
    {
        printf("子进程比较累,先休息两秒钟...\n");
        sleep(2);
        printf("子进程休息好了,太无聊了,就退出了..\n");
        exit(0);
    }
    else
    {
        //父进程
        while (1)
        {
            printf("父进程do working,,.\n");
            sleep(1);
        }
    }
    return 0;
}






//11.2如何避免僵尸进程
//1)最简单的方法, 父进程通过wait和waitpid等函数等待子进程结束, 但是, 这会导致父进程挂起。
//2)如果父进程要处理的事情很多, 不能够挂起, 通过signal函数人为处理信号SIGCHLD, 只要有子进程退出
//自动调用指定好的回调函数, 因为子进程结束后, 父进程会收到该信号SIGCHLD, 可以在其回调函数里调用
//wait()或waitpid回收。


void fun(int signo)
{
    printf("捕捉到信号:%d\n", signo);         //17
    printf("有子进程退出.,,,\n");
    //非阻塞方式，在没有可用的子进程退出时，waitpid 不会阻塞，而是立即返回0
    while ((waitpid(-1, NULL, WNOHANG)) > 0) {}
}

int main(void)
{
    pid_t pid = -1;
    struct sigaction act;
    act.sa_handler = fun;
    act.sa_flags = 0;
    //注册信号处理函数
    sigaction(SIGCHLD, &act, NULL);
    //创建一个子进程
    pid = fork();
    if (-1 == pid)
    {
        perror("fork");
        return 1;
    }
    //子进程
    if (0 == pid)
    {
        printf("子进程比较累,先休息两秒钟...\n");
        sleep(2);
        printf("子进程休息好了,太无聊了,就退出了..\n");
        exit(0);
    }
    else
    {
        //父进程
        while (1)
        {
            printf("父进程do working,,.\n");
            sleep(1);
        }
    }
    return 0;
}
