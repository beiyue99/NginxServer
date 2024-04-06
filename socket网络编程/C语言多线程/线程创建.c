


int pthread_create(pthread_t* thread,
	const pthread_attr_t* attr,
	void* (*start_routine)(void*)
	void* arg)
	功能:
创建一个线程。
参数 :
thread:线程标识符地址。
attr : 线程属性结构体地址, 通常设置为NULL。
start_routine : 线程函数的入口地址。
arg : 传给线程函数的参数。
返回值 :
成功:0
失败 : 非0



pthread_create函数的第二个参数是一个指向pthread_attr_t类型的线程属性对象的指针，用于指定新线程的属性。
如果不需要控制线程的属性，可以将该参数设置为NULL，这样线程将使用POSIX线程库的默认属性值。
例如，下面是使用pthread_create函数创建新线程的示例代码，其中第二个参数为NULL，表示使用默认线程属性：
pthread_t thread;
int rc = pthread_create(&thread, NULL, my_thread_func, arg);
如果需要控制线程的属性，需要先使用pthread_attr_init函数初始化一个线程属性对象，
然后使用其他pthread_attr函数来设置线程属性的值，最后将该线程属性对象作为pthread_create函数的第二个参数传递。
例如，下面是使用pthread_create函数创建新线程的示例代码，其中使用pthread_attr_init函数初始化线程属性对象，
然后使用pthread_attr_setstacksize函数设置线程堆栈的大小，最后将该线程属性对象作为pthread_create函数的第二个参数传递：
pthread_t thread;
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setstacksize(&attr, 2 * PTHREAD_STACK_MIN);
int rc = pthread_create(&thread,
	&attr, my_thread_func, arg);






在一个线程中调用pthread_create创建新的线程后, 当前线程从pthread_create返回继续往下执行, 而新
的线程所执行的代码由我们传给pthread_create的函数指针start_routine决定。
由于pthread_create的错误码不保存在errno中, 因此不能直接用perror()打印错误信息, 可以先用strerror()
把错误码转换成错误信息再打印。



void* fun(void* arg)
{
	printf("新的线程执行任务 tid:%ld\n", pthread_self());
	return NULL;
}
int main()
{
	int ret = -1;
	pthread_t tid = -1;
	//创建一个线程
	ret = pthread_create(&tid, NULL, fun, NULL);
	if (ret != 0)
	{
		printf("pthread_create failed...\n");
		return 1;
	}
	printf("main thread tid is : %ld\n", pthread_self());
	return 0;
}

可能新线程还没有执行，主线程就已经结束，此时不会执行fun函数里面的内容
若想要新线程执行，可以用getchar函数阻塞一下主线程












void* fun(void* arg)
{
	printf("新的线程执行任务 tid:%ld\n", pthread_self());
	return NULL;
}
void* fun1(void* arg)
{
	int var = (int)(long)arg;
	printf("线程2var=%d\n", var);
	return NULL;
}
int main()
{
	int ret = -1;
	pthread_t tid = -1;
	pthread_t tid2 = -1;

	//创建一个线程
	ret = pthread_create(&tid, NULL, fun, NULL);
	if (ret != 0)
	{
		printf("pthread_create failed...\n");
		return 1;
	}
	ret = pthread_create(&tid2, NULL, fun1, (void*)0x3);
	if (ret != 0)
	{
		printf("pthread_create failed...\n");
		return 1;
	}
	printf("main thread tid is : %ld\n", pthread_self());
	printf("按任意键退出\n");
	getchar();
	return 0;
}