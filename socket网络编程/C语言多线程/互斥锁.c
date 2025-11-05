

#include <pthread.h>
int pthread_mutex_init(pthread_mutex_t* restrict mutex,
	const pthread_mutexattr_t* restrict attr)
	功能:
初始化一个互斥锁。
参数 :
mutex:互斥锁地址。类型是pthread_mutex_t
。
attr : 设置互斥量的属性, 通常可采用默认属性, 即可将attr设为NULL。
可以使用宏PTHREAD_MUTEX_INITIALIZER静态初始化互斥锁, 比如 :
	pthread_mutex_t mutex PTHREAD_MUTEX_INITIALIZER;
这种方法等价于使用NULL指定的attr参数调用pthread_mutex_init()来完成动态初始化, 不同之处在
于PTHREAD_MUTEX._INITIALIZER宏不进行错误检查。
返回值 :
成功:0, 成功申请的锁默认是打开的。
失败 : 非0错误码


restrict, C语言中的一种类型限定链(Type Qualifiers), 用于告诉编译器, 对象已经被指针所引用, 不能通
过除该指针外所有其他直接或间接的方式修改该对象的内容。



int pthread_mutex_destroy(pthread_mutex_t* mutex);
功能:
销毁指定的一个互斥锁。互斥锁在使用完毕后, 必须要对互斥锁进行销毁, 以释放资源。
参数 :
mutex:互斥锁地址。
返回值 :
成功:0
失败 : 非0错误码








int pthread_mutex_lock(pthread_mutex_t * mutex);
功能:
对互斥锁上锁, 若互斥锁已经上锁, 则调用者阻塞, 直到互斥锁解锁后再上锁。
参数 :
mutex:互斥锁地址。
返回值 :
成功:0
失败 : 非0错误码
int pthread_mutex_trylock(pthread_mutex_t * mutex);

调用该函数时, 若互斥锁末加锁, 则上锁, 返回0
若互斥锁已力锁, 则函数直接返回失败, 即EBUSY。








int pthread_mutex_unlock(pthread_mutex_t* mutex);
功能:
对指定的互斥锁解锁。
参数 :
mutex:互斥锁地址。
返回值 :
成功:
失败:非0错误码












#include<stdio.h>
#include<unistd.h>
#include<pthread.h>


//互斥锁变量
pthread_mutex_t mutex;


//输出大写字母
void* fun1(void* arg)
{
	int i = 0;
	pthread_mutex_lock(&mutex);
	for (i = 'A'; i <= 'Z'; i++)
	{
		putchar(i);
		fflush(stdout);
		usleep(100000);//100ms
	}
	pthread_mutex_unlock(&mutex);
	return NULL;
}

//输出小写字母
void* fun2(void* arg)
{
	int i = 0;
	//pthread_mutex_lock(&mutex);
	for (i = 'a'; i <= 'z'; i++)
	{
		putchar(i);
		fflush(stdout);   //如果不刷新，就会输出在缓冲区
		usleep(100000);//100ms
	}
	//pthread_mutex_unlock(&mutex);
	return NULL;
}
//模拟输出字符
int main(void)
{

	int ret = -1;
	pthread_t tid1, tid2;
	//初始化一个互斥量 互斥锁
	ret = pthread_mutex_init(&mutex, NULL);
	if (ret != 0)
	{
		printf("pthread_mutex_init failed...\n");
		return 1;
	}
	printf("初始化一个互斥量成功....\n");
	//创建两个线程
	pthread_create(&tid1, NULL, fun1, NULL);
	pthread_create(&tid2, NULL, fun2, NULL);
	//等待两个线程结束
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	printf("\n");
	printf("main thread exit....\n");
	//销毁互斥量 互斥锁
	pthread_mutex_destroy(&mutex);
	return 0;
}

首先，fun1线程会获取互斥锁并开始循环输出大写字母。在循环的每次迭代中，它会输出一个大写字母，
然后调用fflush(stdout); 刷新输出缓冲区，确保字母被立即打印到终端。

然后，fun1线程会调用usleep(100000); 来暂停执行，即睡眠100毫秒（或者说延迟100毫秒）。在这个睡眠期间，fun1线程仍然持有互斥锁。

但是，在fun1线程睡眠期间，操作系统可能会进行线程调度，并有可能切换到fun2线程执行。
由于fun2线程没有互斥锁的保护，它可以独立地执行输出小写字母的操作。

因此，fun1线程和fun2线程在睡眠期间可以交替执行，从而导致大写字母和小写字母交错出现在输出结果中。






当只对fun1线程进行互斥锁的加锁和解锁操作时，fun2线程在fun1线程睡眠期间有机会被系统调度执行，因为fun2线程没有互斥锁的保护。

而当同时对fun1和fun2线程都进行互斥锁的加锁和解锁操作时，一个线程在执行期间，即使另一个线程处于睡眠状态，它也无法获取到互斥锁，因此无法执行自己的任务。

因此，在同时对fun1和fun2线程都加锁的情况下，只有当一个线程执行完毕并释放互斥锁后，另一个线程才能获取到互斥锁并执行自己的任务。










//死锁案例
#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;

//线程1处理函数
void* fun1(void* arg)
{
	//线程1先申请资源1再申请资源2
	pthread_mutex_lock(&mutex1);
	printf("线程1锁资源1ok..,.\n");
	pthread_mutex_lock(&mutex2);
	printf("线程1锁资源2ok..,.\n");
	printf("线程1执行临界区代码..\n");
	//解锁互斥锁
	pthread_mutex_unlock(&mutex1);
	pthread_mutex_unlock(&mutex2);
	return NULL;
}
//线程2处理函数
void* fun2(void* arg)
{
	//线程2先申请资源2再申请资源1
	pthread_mutex_lock(&mutex2);
	printf("线程2加锁资源2ok...\n");
	pthread_mutex_lock(&mutex1);
	printf("线程2加锁资源1ok...\n");
	printf("线程2执行临界区代码....\n");
	//解锁互斥锁
	pthread_mutex_unlock(&mutex2);
	pthread_mutex_unlock(&mutex1);
	return NULL;
}
int main(void)
{
	int ret = -1;
	pthread_t tid1, tid2;
	//初始化互斥量
	pthread_mutex_init(&mutex1, NULL);
	pthread_mutex_init(&mutex2, NULL);
	//创建两个线程
	pthread_create(&tid1, NULL, fun1, NULL);
	pthread_create(&tid2, NULL, fun2, NULL);
	//回收线程资源
	ret = pthread_join(tid1, NULL);
	if (0 != ret)
	{
		("pthread_join failed....In");
		return 1;
	}
	ret = pthread_join(tid2, NULL);
	if (0 != ret)
	{
		printf("pthread_join failed....\n");
		return 1;
	}
	//销毁
	pthread_mutex_destroy(&mutex1);
	pthread_mutex_destroy(&mutex2);
	return 0;
}
