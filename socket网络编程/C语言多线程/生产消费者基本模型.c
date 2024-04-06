
1.如果产品链表为空, 那么消费者就会阻塞, 等待链表不为空
2.如果产品链表不为空, 那么消费者就可以直接消费



//生产者和消费者模型条件变量的模型




//节点结构
typedef struct node
{
	int data;
	struct node* next;
}Node;
//永远指向链表头部的指针
Node* head = NULL;
//线程同步-互斥锁
pthread_mutex_t mutex;
//阻塞线程~条件变量类型的变量
pthread_cond_t cond;
//生产者
void* producer(void* arg)
{
	while (1)
	{
		//创建一个链表的节点
		Node* pnew = (Node*)malloc(sizeof(Node));
		memset(pnew, 0, sizeof(Node));
		//节点的初始化
		pnew->data = rand() % 1000;//0-999
		pnew->next = NULL;
		//使用互斥锁保护共享数据
		pthread_mutex_lock(&mutex);
		//头插法
		pnew->next = head;
		head = pnew;
		printf("======produce:%lu,%d\n", pthread_self(), pnew->data);
		pthread_mutex_unlock(&mutex);
		//通知阻塞的消费者线程,解除阻塞
		pthread_cond_signal(&cond);
		sleep(rand() % 2);
	}
	return NULL;
}

void* customer(void* arg)
{
	while (1)
	{
		pthread_mutex_lock(&mutex);
		//判断链表是否为空
		if (head == NULL)
		{
			//线程阻塞
			//该函数会对互斥锁解锁
			pthread_cond_wait(&cond, &mutex);
			//解除阻塞之后,对互斥锁做加锁操作
		}
		//链表不为空一删掉一个节点~删除头结点
		Node* pdel = head;
		head = head->next;
		printf("------customer:%lu,%d\n", pthread_self(), pdel->data);
		free(pdel);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}


int main(void)
{
	pthread_t tid1 = -1, tid2 = -1;
	int ret = 0;
	//设置随机种子
	srandom(getpid());
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
	//创建两个线程生产者线程
	//一个消费者线程
	pthread_create(&tid1, NULL, producer, NULL);
	//创建消费者线程
	pthread_create(&tid2, NULL, customer, NULL);
	//等待两个线程结束
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	//销毁
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
	return 0;
}