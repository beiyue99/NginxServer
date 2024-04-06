





条件变量概述
与互斥锁不同, 条件变量是用来等待而不是用来上锁的, 条件变量本身不是锁!
条件变量用来自动阻塞一个线程, 直到某特殊情况发生为止。通常条件变量和互斥锁同时使用。
条件变量的两个动作 :
·条件不满, 阻塞线程
·当条件满足, 通知阻塞的线程开始工作
条件变量的类型 : pthread_cond_t。





int pthread_cond_init(pthread_cond_t* restrict cond, const pthread_condattr_t* restrict attr);
功能:
初始化一个条件变量
参数 :
cond:指向要初始化的条件变量指针。
attr : 条件变量属性, 通常为默认值, 传NULL即可
也可以使用静态初始化的方法, 初始化条件变量 :
	pthread_cond_t cond PTHREAD_COND_INITIALIZER;
返回值:
成功:0
失败 : 非0错误号



int pthread_cond_destroy(pthread_cond_t * cond);
功能:
销毁一个条件变量
参数 :
cond:指向要初始化的条件变量指针
返回值 :
成功:0
失败 : 非0错误号





int pthread_cond_wait(pthread_cond_t * restrict cond, pthread_mutex_t * restrict mutex);
功能:
阻塞等待一个条件变量
a)阻塞等待条件变量cond(参1)满足
b)释放已掌握的互斥锁(解锁互斥量)相当于pthread_mutex_unlock(&mutex);
a)b)两步为一个原子操作。
c)当被唤醒, pthread_cond_wait函数返回时, 解除阻塞并重新申请获取互斥锁  pthread_mutex_lock(&mutex);
参数:
cond:指向要初始化的条件变量指针
mutex : 互斥锁
返回值 :
成功:0
失败 : 非0错误号





int pthread_cond_signal(pthread_cond_t * cond);
功能:
唤醒至少一个阻塞在条件变量上的线程
参数 :
cond:指向要初始化的条件变量指针
返回值 :
成功:0
失败 : 非0错误号
int pthread_cond_broadcast(pthread_cond_t * cond);
功能:
唤醒全部阻塞在条件变量上的线程
参数 :
cond:指向要初始化的条件变量指针
返回值 :
成边:0
失败 : 非0错误号





int flag = 0;
//互斥量
pthread_mutex_t mutex;
//条件变量
pthread_cond_t cond;
//改变条件的线程
void* fun1(void* arg)
{
	while (1)
		//加锁
	{
		pthread_mutex_lock(&mutex);
		flag = 1;
		//解锁
		pthread_mutex_unlock(&mutex);
		//唤醒因为条件而阻塞线程
		pthread_cond_signal(&cond);
		sleep(1);
	}
	return NULL;
}
//等待条件的线程
void* fun2(void* arg)
{
	while (1)
	{
		//加锁
		pthread_mutex_lock(&mutex);
		//表示条件不满足
		if (0 != flag)
			//等待条件满足,同时会阻塞
		{
			pthread_cond_wait(&cond, &mutex);//后续收到信号，解除阻塞
			printf("线程二因为条件满足开始运行...\n");
		}
		flag = 0;
		//解锁
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}


int main(void)
{
	int ret = -1;
	pthread_t tid1, tid2;
	//初始化条件变量
	ret = pthread_cond_init(&cond, NULL);
	if (0 != ret)
	{
		printf("pthread_cond_init failed...\n");
		return 1;
	}
	//初始化互斥量
	ret = pthread_mutex_init(&mutex, NULL);
	if (0 != ret)
	{
		printf("pthread_mutex_init failed...\n");
		return 1;
	}
	pthread_create(&tid1, NULL, fun1, NULL);
	pthread_create(&tid2, NULL, fun2, NULL);
	ret = pthread_join(tid1, NULL);
	if (0 != ret)
	{
		printf("pthread join failed...\n");
		return 1;
	}
	ret = pthread_join(tid2, NULL);
	if (0 != ret)
	{
		printf("pthread_join failed...\n");
		return 1;
	}
	//销毁互斥量
	pthread_mutex_destroy(&mutex);
	//销毁条件变量
	pthread_cond_destroy(&cond);
	return 0;
}









