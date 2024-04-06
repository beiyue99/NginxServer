当一个读线程加锁, 其它所有读线程加锁都能0k所有写线程都会阻塞
当一个写线程加锁, 其它所有的读线程和写线程加锁都会阻塞


读写锁分为读锁和写锁, 规则如下:
1)如果某线程申请了读锁, 其它线程可以再申请读锁, 但不能申请写锁。
2)如果某线程申请了写锁, 其它线程不能申请读锁, 也不能申请写锁。




int pthread_rwlock_int(pthread_rwlock_t* restrict rwlock, const pthread_rwlockattr_t* restrict attr);
功能:
用来初始化rw1ock所指向的读写锁。
参数 :
rwlock:指向要初始化的读写锁指针。
attr : 读写锁的属性指针。如果attr为NULL则会使用默认的属性初始化读写锁, 否则使用指定的attr初始化读写锁。
可以使用宏PTHREAD_RWLOCK_INITIALIZER静态初始化读写锁, 比如 :
	pthread_rwlock_t my_rwlock PTHREAD_RWLOCK_INITIALIZER;
这种方法等价于使用NULL指定的attr参数调用pthread_rwlock_init()来完成动态初始化, 不同之处在于PTHREAD_RWLOCK._INITIALIZER宏不进行错误检查。
返回值 :
成功:0, 读写锁的状态将成为已初始化和已解锁。
失败 : 非0错误码。




int pthread_rwlock_destroy(pthread_rwlock_t* rwlock);
功能:
用于销毁一个读写锁, 并释放所有相关联的资源(所谓的所有指的是由pthread_rwlock_init()自动申请的资源)。
参数 :
rwlock:读写锁指针。
返回值 :
成功:0
失败 : 非0错误码







int pthread_rwlock_rdlock(pthread_rwlock_t * rwlock);
功能:
以阻塞方式在读写锁上获取读锁(读锁定)。
如果没有写者持有该锁, 并且没有写者阻塞在该锁上, 则调用线程会获取读锁。
如果调用线程未获取读锁, 则它将阻塞直到它获取了该锁。一个线程可以在一个读写锁上多次执行读锁定。
线程可以成功调用pthread._rw1ock_rdlock()函数n次, 但是之后该线程必须调用
pthread_rwlock_unlock()函数n次才能解除锁定。
参数 :
rwlock:读写锁指针。
返回值 :
成功:0
失败 : 非0错误码
int pthread_rwlock_tryrdlock(pthread_rwlock_t * rwlock);
用于尝试以非阻塞的方式来在读写锁上获取读锁。
如果有任何的写者持有该锁或有写者阻塞在该读写锁上, 则立即失败返回。








int pthread_rwlock_wrlock(pthread_rwlock_t* rwlock);
功能:
在读写锁上获取写锁(写锁定)。
如果没有写者持有该锁, 并且没有读者持有该锁, 则调用线程会获取写锁 :
	如果调用线程未获取写锁, 则它将阻塞直到它获取了该锁。
	参数 :
rw1ock:读写锁指针。
返回值 :
成功:0
失败 : 非0错误码
int pthread_rwlock_trywrlock(pthread_rwlock_t * rwlock);
用于尝试以非阻塞的方式来在读写锁上获取写锁。
如果有任何的读者或写者持有该锁, 则立即失败返回。





int pthread_rwlock_unlock(pthread_rwlock_t* rwlock);
功能:
无论是读锁或写锁, 都可以通过此函数解锁。
参数 :
rwlock:读写锁指针。
返回值 :
成功:0
失败 : 非0错误码





//全局变量
int num = 0;
//读写锁变量
pthread_rwlock_t rwlock;
//读线程
void* fun_read(void* arg)
{
	//获取线程的编号
	int index = (int)(intptr_t)arg;
	while (1)
	{
		//加读写锁读锁
		pthread_rwlock_rdlock(&rwlock);
		printf("线程%d读取num的值%d\n", index, num);
		//解锁
		pthread_rwlock_unlock(&rwlock);
		//随机睡眠1-3秒
		sleep(random() % 3 + 1);
	}
	return NULL;
}

//写线程
void* fun_write(void* arg)
{
	int index = (int)(intptr_t)arg;
	while (1)
	{
		//加读写锁写锁
		pthread_rwlock_wrlock(&rwlock);
		num++;
		printf("线程%d修改num的值%d\n", index, num);
		//解锁
		pthread_rwlock_unlock(&rwlock);
		//随机睡眠1-3秒
		sleep(random() % 3 + 1);
	}
	return NULL;
}
int main(void)
{
	int i = 0;
	int ret = -1;
	pthread_t tid[8];
	//设置随机种子
	srandom(getpid());
	//初始化读写锁
	ret = pthread_rwlock_init(&rwlock, NULL);
	if (0 != ret)
	{
		printf("pthread_rwlock_init failed...\n");
		return 1;
	}
	//创建8个线程
	for (i = 0; i < 8; i++)
	{
		//创建读线程
		if (i < 5)
		{
			pthread_create(&tid[i], NULL, fun_read, (void*)(intptr_t)i);
		}
		else
		{
			//创建写线程
			pthread_create(&tid[i], NULL, fun_write, (void*)(intptr_t)i);
		}
	}
	//回收八个线程的资原
	for (i = 0; i < 8; i++)
	{
		pthread_join(tid[i], NULL);
	}
	//销毁卖写锁
	pthread_rwlock_destroy(&rwlock);
	return 0;
}

//读取数据不会不一致，如果不上锁，则不行
线程0读取num的值0
线程2读取num的值0
线程1读取num的值0
线程3读取num的值0
线程4读取num的值0
线程5修改num的值1
线程6修改num的值2
线程7修改num的值3
线程2读取num的值3
线程4读取num的值3
线程1读取num的值3