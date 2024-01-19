
//alarm函数
int alarm(int seconds);
设置闹钟，在指定second之后，内核会给当前进程发送14（SIGALRM)的信号，进程收到该信号默认动作终止，一个进程只有唯一的一个定时器。
alarm(0) : 取消闹钟定时器，返回余下的秒数




8 int main()
9 {
    10     unsigned int ret = 0;
    11     //设置闹钟，5秒钟之后就超时，发送对应的信号
        12     ret = alarm(5);
    13     printf("上一次闹钟剩下的秒数：%u\n", ret);
    14     sleep(2);
    15     //旧闹钟被新闹钟覆盖
        16     ret = alarm(4);
    17     printf("上一次闹钟剩下的时间是：%u\n", ret);   返回3
        18     printf("按下任意键继续...\n");
    19     getchar();
    20     return 0;
    21 }









setitimer函数  int setitimer(int which, const struct itimerval* new_value, struct itimerval* old_value);
功能：设置闹钟定时器，可代替alarm函数，精度微秒us，可以实现周期定时
which : 指定定时方式：
1.自然定时：ITIMER_REAL->14)SIGALRM计算自然时间
2.虚拟空间计时（用户空间）:ITIMER_VIRTUAL->26)SIGVTALRM 只计算进程占用cpu时间
3.运行时计时（用户 + 内核）:ITIMER_PROF->27)SIGPROF计算占用cpu以及执行系统调用的时间
new_value : struct itimerval, 负责设定timeout时间
struct itimerval
{
    struct timerval it_interval; //闹钟触发周期     设定以后每几秒执行
    struct timerval it_value;    //闹钟触发时间     设定第一次执行后所延迟的秒数
};
struct timeval
{
    long tv_sec;  //秒
    long tv_usec  //微秒
};
old_value：存放旧的timeout值，一般为NULL;
返回值：成功返回0失败返回 - 1




定时器一旦触发，闹钟超时会向内核发送一个信号，该信号默认动作是终止进程。所以计时器只会触发一次
//可以通过信号捕捉函数，使该函数失效




int main()
{
    int ret = -1;
    struct itimerval tmo = { 0 };
    //第一次触发时间
    tmo.it_value.tv_sec = 3;
    tmo.it_value.tv_usec = 0;
    //触发周期
    tmo.it_interval.tv_sec = 2;
    tmo.it_interval.tv_usec = 0;
    //设置定时器
    ret = setitimer(ITIMER_REAL, (const struct itimerval*)&tmo, NULL);

    if (ret == -1)
    {
        perror("setitimer");
        return 1;
    }
    printf("按下任意键继续\n");
    getchar();
    return 0;
}