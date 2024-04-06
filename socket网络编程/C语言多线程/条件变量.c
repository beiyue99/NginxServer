





������������
�뻥������ͬ, ���������������ȴ�����������������, ����������������!
�������������Զ�����һ���߳�, ֱ��ĳ�����������Ϊֹ��ͨ�����������ͻ�����ͬʱʹ�á�
������������������ :
����������, �����߳�
������������, ֪ͨ�������߳̿�ʼ����
�������������� : pthread_cond_t��





int pthread_cond_init(pthread_cond_t* restrict cond, const pthread_condattr_t* restrict attr);
����:
��ʼ��һ����������
���� :
cond:ָ��Ҫ��ʼ������������ָ�롣
attr : ������������, ͨ��ΪĬ��ֵ, ��NULL����
Ҳ����ʹ�þ�̬��ʼ���ķ���, ��ʼ���������� :
	pthread_cond_t cond PTHREAD_COND_INITIALIZER;
����ֵ:
�ɹ�:0
ʧ�� : ��0�����



int pthread_cond_destroy(pthread_cond_t * cond);
����:
����һ����������
���� :
cond:ָ��Ҫ��ʼ������������ָ��
����ֵ :
�ɹ�:0
ʧ�� : ��0�����





int pthread_cond_wait(pthread_cond_t * restrict cond, pthread_mutex_t * restrict mutex);
����:
�����ȴ�һ����������
a)�����ȴ���������cond(��1)����
b)�ͷ������յĻ�����(����������)�൱��pthread_mutex_unlock(&mutex);
a)b)����Ϊһ��ԭ�Ӳ�����
c)��������, pthread_cond_wait��������ʱ, ������������������ȡ������  pthread_mutex_lock(&mutex);
����:
cond:ָ��Ҫ��ʼ������������ָ��
mutex : ������
����ֵ :
�ɹ�:0
ʧ�� : ��0�����





int pthread_cond_signal(pthread_cond_t * cond);
����:
��������һ�����������������ϵ��߳�
���� :
cond:ָ��Ҫ��ʼ������������ָ��
����ֵ :
�ɹ�:0
ʧ�� : ��0�����
int pthread_cond_broadcast(pthread_cond_t * cond);
����:
����ȫ�����������������ϵ��߳�
���� :
cond:ָ��Ҫ��ʼ������������ָ��
����ֵ :
�ɱ�:0
ʧ�� : ��0�����





int flag = 0;
//������
pthread_mutex_t mutex;
//��������
pthread_cond_t cond;
//�ı��������߳�
void* fun1(void* arg)
{
	while (1)
		//����
	{
		pthread_mutex_lock(&mutex);
		flag = 1;
		//����
		pthread_mutex_unlock(&mutex);
		//������Ϊ�����������߳�
		pthread_cond_signal(&cond);
		sleep(1);
	}
	return NULL;
}
//�ȴ��������߳�
void* fun2(void* arg)
{
	while (1)
	{
		//����
		pthread_mutex_lock(&mutex);
		//��ʾ����������
		if (0 != flag)
			//�ȴ���������,ͬʱ������
		{
			pthread_cond_wait(&cond, &mutex);//�����յ��źţ��������
			printf("�̶߳���Ϊ�������㿪ʼ����...\n");
		}
		flag = 0;
		//����
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}


int main(void)
{
	int ret = -1;
	pthread_t tid1, tid2;
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
	//���ٻ�����
	pthread_mutex_destroy(&mutex);
	//������������
	pthread_cond_destroy(&cond);
	return 0;
}









