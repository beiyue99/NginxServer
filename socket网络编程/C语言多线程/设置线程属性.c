int pthread_attr_setdetachstate(pthread_attr_t* attr, int detachstate);
����:�����̷߳���״̬
���� :
attr:�ѳ�ʼ�����߳�����
detachstate :
����״̬
PTHREAD_CREATE_DETACHED(�����߳�)
PTHREAD_CREATE_JOINABLE(�Ƿ����߳�)
����ֵ :
	�ɹ� : 0
	ʧ�� : ��0

	�˺��������ڴ����߳�ǰʹ�ã��������̵߳ķ���״̬�������߳̽������Ƿ��Զ��ͷ���Դ��
	�������Ϊ����״̬�����߳̽�������Դ���Զ��ͷţ�������Ҫͨ�� pthread_join() �����ȴ��߳̽�����������Դ��









	����ԭ�ͣ�int pthread_attr_getdetachstate(const pthread_attr_t * attr, int* detachstate);

�������ã���ȡ�߳����Բ����еķ���״̬��

����������

attr : ָ�� pthread_attr_t ���͵�ָ�룬����ָ��Ҫ��ȡ���߳����Բ�����
detachstate : ָ�� int ���͵�ָ�룬���ڴ洢��ȡ�����̷߳���״̬��
��������ֵ��

�ɹ������� 0��
ʧ�ܣ����ش����롣
�˺��������ڴ����̺߳�ʹ�ã��Ի�ȡ�̵߳ķ���״̬���Ծ����Ƿ���Ҫ���� pthread_join() �����������߳���Դ������̴߳��ڷ���״̬�����޷�ʹ�� pthread_join() �����ȴ��߳̽�����










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
	//��ʼ���߳�����
	pthread_attr_t attr;
	ret = pthread_attr_init(&attr);
	if (ret != 0)
	{
		printf("pthread init faile..\n");
		return 1;
	}
	printf("pthread init ok..\n");
	//�����߳�����Ϊ����״̬,��Ϊ�ڶ�����������
	ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (ret != 0)
	{
		printf("pthread set failed...\n");
		return 1;
	}
	//����һ���߳�
	ret = pthread_create(&tid, &attr, fun, NULL);
	if (0 != ret)
	{
		printf("pthread_create failed....\n");
		return 1;
	}
	//�����Ƿ�Ϊ����״̬
	ret = pthread_join(tid, NULL);
	if (ret != 0)
	{
		printf("��ǰ�߳�Ϊ����״̬\n");
	}
	else
	{
		printf("��ǰ�߳�Ϊ�Ƿ���״̬\n");
	}
	//�����߳�����
	ret = pthread_attr_destroy(&attr);
	if (ret != 0)
	{
		printf("pthread destroy failed..\n");
		return 0;
	}
	printf("����������˳�..\n");
	getchar();
	return 0;
}

pthread init ok..
��ǰ�߳�Ϊ����״̬
����������˳�..
fun thread do working 0
fun thread do working 1
fun thread do working 2
fun thread do working 3
fun thread do working 4