

//假设sem1代表商品容器，初始值为4，每生产一个商品，容器就会减一，
//sem2代表商品，初始值为0，每消耗一个商品，容器就会加一




//节点结构
typedef struct node
{
	int data;
	struct node* next;
}Node;
//永远指向链表头部的指针
Node* head = NULL;
//容器的个数
sem_t sem_producer;
//可以卖产品的个数
sem_t sem_customer;
//生产者
void* producer(void* arg)
{
	Node* pnew = NULL;
	//循环生产产品
	while (1)
	{
		//申请一个资源容器
		sem_wait(&sem_producer);
		//分配节点空间
		pnew = malloc(sizeof(Node));
		if (NULL == pnew)
		{
			printf("malloc failed....\n");
			break;
		}
		memset(pnew, 0, sizeof(Node));
		//赋值
		pnew->data = random() % 100 + 1;
		pnew->next = NULL;
		printf("生产者生产产品%d\n", pnew->data);
		//头插法
		pnew->next = head;
		head = pnew;
		//通知消费者消费将可以卖的商品个数加1
		sem_post(&sem_customer);
		sleep(random() % 3 + 1);
	}
	return NULL;
}


//消费者
void* customer(void* arg)
{
	Node* tmp = NULL;
	while (1)
	{
		//申请资源可以卖的商品个数减1
		sem_wait(&sem_customer);
		//链表为空的情形
		if (NULL == head)
		{
			printf("产品链表为空..,.先休息2秒钟..n");
		}
		//第一个节点地址赋值给临时变量tmp
		tmp = head;
		//head指向链表的第二个节点
		head = head->next;
		printf("消费者消耗产品%d\n", tmp->data);
		//释放空间
		free(tmp);
		//释放资源将容器个数加1
		sem_post(&sem_producer);
		//睡眠1-3秒
		sleep(random() % 3 + 1);
	}
	return NULL;
}

//没有使用条件变量的生产者和消费者模型
int main(void)
{
	int ret = -1;
	pthread_t tid1, tid2;
	//设置随机种子
	srandom(getpid());
	//初始化
	ret = sem_init(&sem_producer, 0, 4);
	if (0 != ret)
	{
		printf("sem_init failed...\n");
		return 1;
	}
	//初始化可以卖的商品的个数
	ret = sem_init(&sem_customer, 0, 0);
	if (0 != ret)
	{
		printf("sem_init failed...\n");
		return 1;
	}
	//创建两个线程
	pthread_create(&tid1, NULL, producer, NULL);
	pthread_create(&tid2, NULL, customer, NULL);
	//回收线程资源
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	//销毁
	sem_destroy(&sem_producer);
	sem_destroy(&sem_customer);
	return 0;
}