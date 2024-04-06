
1.�����Ʒ����Ϊ��, ��ô�����߾ͻ�����, �ȴ�����Ϊ��
2.�����Ʒ����Ϊ��, ��ô�����߾Ϳ���ֱ������



//�����ߺ�������ģ������������ģ��




//�ڵ�ṹ
typedef struct node
{
	int data;
	struct node* next;
}Node;
//��Զָ������ͷ����ָ��
Node* head = NULL;
//�߳�ͬ��-������
pthread_mutex_t mutex;
//�����߳�~�����������͵ı���
pthread_cond_t cond;
//������
void* producer(void* arg)
{
	while (1)
	{
		//����һ������Ľڵ�
		Node* pnew = (Node*)malloc(sizeof(Node));
		memset(pnew, 0, sizeof(Node));
		//�ڵ�ĳ�ʼ��
		pnew->data = rand() % 1000;//0-999
		pnew->next = NULL;
		//ʹ�û�����������������
		pthread_mutex_lock(&mutex);
		//ͷ�巨
		pnew->next = head;
		head = pnew;
		printf("======produce:%lu,%d\n", pthread_self(), pnew->data);
		pthread_mutex_unlock(&mutex);
		//֪ͨ�������������߳�,�������
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
		//�ж������Ƿ�Ϊ��
		if (head == NULL)
		{
			//�߳�����
			//�ú�����Ի���������
			pthread_cond_wait(&cond, &mutex);
			//�������֮��,�Ի���������������
		}
		//����Ϊ��һɾ��һ���ڵ�~ɾ��ͷ���
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
	//�����������
	srandom(getpid());
	//��ʼ����������
	ret = pthread_cond_init(&cond, NULL);
	if (0 != ret)
	{
		printf("pthread_cond_init failed...\n");
		return 1;
	}
	//��ʼ��������
	ret = pthread_mutex_init(&mutex, NULL);
	if (0 != ret)
	{
		printf("pthread_mutex_init failed...\n");
		return 1;
	}
	//���������߳��������߳�
	//һ���������߳�
	pthread_create(&tid1, NULL, producer, NULL);
	//�����������߳�
	pthread_create(&tid2, NULL, customer, NULL);
	//�ȴ������߳̽���
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);
	//����
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
	return 0;
}