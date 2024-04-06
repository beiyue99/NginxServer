
线程共享资源
1)文件描述符表
2)每种信号的处理方式
3)当前工作目录
4)用户ID和组ID
内存地址空间(.text / .data / .bss / heap / 共享库)

线程非共享资源
1)线程id
2)处理器现场和栈指针(内核栈)
3)独立的栈空间(用户空间栈)
4)errno变量
5)信号屏蔽字
6)调度优先级




memset(&tid, 0, sizeof(tid));    如果不知道 tid是什么类型，可以这样初始化







int num = 100;
void* fun(void* arg)
{
	printf("brfore num is %d\n", num);
	num++;
	printf("after num is %d\n", num);
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

	//创建一个线程
	ret = pthread_create(&tid, NULL, fun, NULL);
	if (ret != 0)
	{
		printf("pthread_create failed...\n");
		return 1;
	}
	printf("按任意键继续...\n");
	getchar();
	printf("main num is %d\n", num);
	return 0;
}


子线程中num++，num变为101，最后主线程打出来也是101












#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>


int num = 100;
void* fun(void* arg)
{
	int* pn = (int*)arg;
	printf("brfore num is %d\n", num);
	num++;
	(*pn)++;
	printf("p指向的堆区的值加一后的结果是%d\n", *pn);
	printf("after num is %d\n", num);
	return NULL;
}
int main()
{
	int* p;
	int ret = -1;
	pthread_t tid = -1;
	p = malloc(sizeof(int));
	memset(p, 0, sizeof(int));	*p = 88;

	//创建一个线程
	ret = pthread_create(&tid, NULL, fun, (void*)p);
	if (ret != 0)
	{
		printf("pthread_create failed...\n");
		return 1;
	}
	printf("按任意键继续...\n");
	getchar();
	printf("main num is %d\n", num);
	printf("子线程结束后，主线程*p的值：%d\n", *p);

	return 0;
}

按任意键继续...
brfore num is 100
p指向的堆区的值加一后的结果是89
after num is 101

main num is 101
子线程结束后，主线程 * p的值：89