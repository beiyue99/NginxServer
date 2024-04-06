int pthread_attr_setdetachstate(pthread_attr_t* attr, int detachstate);
功能:设置线程分离状态
参数 :
attr:已初始化的线程属性
detachstate :
分离状态
PTHREAD_CREATE_DETACHED(分离线程)
PTHREAD_CREATE_JOINABLE(非分离线程)
返回值 :
	成功 : 0
	失败 : 非0

	此函数可以在创建线程前使用，以设置线程的分离状态，决定线程结束后是否自动释放资源。
	如果设置为分离状态，则线程结束后资源将自动释放，而不需要通过 pthread_join() 函数等待线程结束并回收资源。









	函数原型：int pthread_attr_getdetachstate(const pthread_attr_t * attr, int* detachstate);

函数作用：获取线程属性参数中的分离状态。

函数参数：

attr : 指向 pthread_attr_t 类型的指针，用于指定要获取的线程属性参数。
detachstate : 指向 int 类型的指针，用于存储获取到的线程分离状态。
函数返回值：

成功：返回 0。
失败：返回错误码。
此函数可以在创建线程后使用，以获取线程的分离状态，以决定是否需要调用 pthread_join() 函数来回收线程资源。如果线程处于分离状态，则无法使用 pthread_join() 函数等待线程结束。










void* fun(void* arg)
{
	int i = 0;
	for (i = 0; i < 5; i++)
	{
		printf("fun thread do working %d\n", i);
		sleep(1);
	}
	pthread_exit(NULL);
}
int main(void)
{
	int ret = -1;
	void* retp = NULL;
	pthread_t tid = -1;
	//初始化线程属性
	pthread_attr_t attr;
	ret = pthread_attr_init(&attr);
	if (ret != 0)
	{
		printf("pthread init faile..\n");
		return 1;
	}
	printf("pthread init ok..\n");
	//设置线程属性为分离状态,作为第二个参数传入
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (ret != 0)
	{
		printf("pthread set failed...\n");
		return 1;
	}
	//创建一个线程
	ret = pthread_create(&tid, &attr, fun, NULL);
	if (0 != ret)
	{
		printf("pthread_create failed....\n");
		return 1;
	}
	//测试是否为分离状态
	ret = pthread_join(tid, NULL);
	if (ret != 0)
	{
		printf("当前线程为分离状态\n");
	}
	else
	{
		printf("当前线程为非分离状态\n");
	}
	//销毁线程属性
	ret = pthread_attr_destroy(&attr);
	if (ret != 0)
	{
		printf("pthread destroy failed..\n");
		return 0;
	}
	printf("按下任意键退出..\n");
	getchar();
	return 0;
}

pthread init ok..
当前线程为分离状态
按下任意键退出..
fun thread do working 0
fun thread do working 1
fun thread do working 2
fun thread do working 3
fun thread do working 4