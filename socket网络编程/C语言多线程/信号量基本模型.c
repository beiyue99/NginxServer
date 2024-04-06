6.1信号量概述
信号量广泛用于进程或线程间的同步和互斥, 信号量本质上是一个非负的整数计数器, 它被用来控制对公共
资源的访问。
编程时可根据操作信号量值的结果判断是否对公共资源具有访问的权限, 当信号量值大于0时, 则可以访
问, 否则将阻塞。
PV原语是对信号量的操作, 一次P操作使信号量减1, 一次V操作使信号量加1。
信号量主要用于进程或线程间的同步和互斥这两种典型情况。
信号量数据类型为 : sem_t






int sem_init(sem_t* sem, int pshared, unsigned int value);
功能:
创建一个信号量并初始化它的值。一个无名信号量在被使用前必须先初始化。
参数 :
sem:
信号量的地址。
pshared : 等于0, 信号量在线程间共享(常用); 不等于0, 信号量在进程间共享。
value : 信号量的初始值。
返回值 :
成功:0
失败 : -1




int sem_wait(sem_t * sem);
功能:
将信号量的值减1。操作前, 先检查信号量(sem)的值是否为0, 若信号量为0, 此函数会阻塞, 直到信号
量大于0时才进行减1操作。
参数 :
sem:信号量的地址。
返回值 :
成功:0
失败 : -1
int sem_trywait(sem_t * sem);
以非阻塞的方式来对信号量进行减1操作。
若操作前, 信号量的值等于0, 则对信号量的操作失败, 函数立即返回。








int sem_getvalue(sem_t* sem, int* sval);
功能:
获取sem标识的信号量的值, 保存在sva1中。
参数 :
sem:信号量地址。
sVa1 : 保存信号量值的地址。
返回值 :
成功:0
失败 : -1









//信号量变量
sem_t sem;
//输出大写字母
void* fun1(void* arg)
{
	int i = 0;
	//申请资源,将可用资源减1
	sem_wait(&sem);
	for (i = 'A'; i <= 'Z'; i++)
	{
		putchar(i);
		fflush(stdout);
		usleep(100000);//100ms
	}
	//释放资源将可用资源加1
	sem_post(&sem);
	return NULL;
}




//输出小写字
void* fun2(void* arg)
{
	int i = 0;
	//申请资源,将可用资源减1
	sem_wait(&sem);
	for (i = 'a'; i <= 'z'; i++)
	{
		putchar(i);
		fflush(stdout);
		usleep(100000);//100ms
	}
	//释放资源将可用资源加1
	sem_post(&sem);
	return NULL;
}

//模拟输出字符
int main(void)
{
	int ret = -1;
	pthread_t tid1, tid2;
	//初始化一个信号量
	ret = sem_init(&sem, 0, 1);
	if (0 != ret)
	{
		printf("sem_init failed...\n");
		return 1;
	}
	printf("初始化一个信号量ok....\n");
	//创建两个线程
	pthread_create(&tid1, NULL, fun1, NULL);
	pthread_create(&tid2, NULL, fun2, NULL);
	//等待两个线程结束
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	printf("\n");
	printf("main thread exit....\n");
	//销毁信号量
	sem_destroy(&sem);
	return 0;
}