#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
using namespace std;
#include<thread>
#include<list>
#include<mutex>
#include <condition_variable>
class A
{
public:
	void inMsgList()
	{
		for (int i = 0; i < 100000; i++)
		{
			cout << "inMsgList����ִ��?�?" << "����һ��Ԫ��:" << " " << i << endl;
			my_mutex1.lock();
			msgRecvList.push_back(i);
			my_cond.notify_one();       //����wait
			my_mutex1.unlock();
		}
	}
	void outMsgList()
	{
		int command = 0;
		while (1)
		{
			unique_lock<mutex>sbguard1(my_mutex1);
			my_cond.wait(sbguard1, [this]
				//������������ڶ�����������false��wait������������������������?�ֱ�������̵߳���notify_one���ٴγ�������������ֹ����
		//���ûд�ڶ�����������ôĬ�Ϸ���false
				{
					if (!msgRecvList.empty())   //������ٻ���?
					{
						return true;
					}
					return false;
				});
			command = msgRecvList.front();
			msgRecvList.pop_front();
			cout << "out������ʼִ��?���Ҫ�?����Ԫ��" << command << "�߳�idΪ��" << this_thread::get_id() << endl;
			sbguard1.unlock();//���������߼����Ƚ��� �Ա������߳̿���ִ��  ����Ҫע����ʱ�����Ѿ��⿪����һ���߳̿����ٴε��û��Ѻ���
			//���Ǵ�ʱ���߳̿�������æ�Ŵ��������߼�����δ���������Ըôλ�����Ч
		}
	}
private:
	list<int > msgRecvList;
	mutex my_mutex1;
	condition_variable my_cond;
};
int  main()
{
	A  a;
	thread myInObj(&A::inMsgList, &a);
	thread myOutObj(&A::outMsgList, &a);
	thread myOutObj2(&A::outMsgList, &a);
	myInObj.join();
	myOutObj.join();
	myOutObj2.join();
}