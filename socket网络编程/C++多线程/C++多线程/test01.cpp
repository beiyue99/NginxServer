#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
using namespace std;
#include<thread>
#include<vector>
#include<future>


#include<list>
#include<mutex>
#include<windows.h>


//  time_mutex   ��mutex�����϶���������Ա����
//  try_lock_for() �ȴ�һ��ʱ�䣬��������
//  try_lock_until �ȵ�δ����ʱ��㣬��������ʱ��֮ǰ�õ������������ߣ�ʱ�㵽�ˣ�û�õ���Ҳ��������

//
//class A
//{
//public:
//	void inMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "inMsgList����ִ�У�" << endl;
//			chrono::milliseconds dura(100);
//			if (my_mutex.try_lock_for(dura))
//			{
//				cout << "����һ��Ԫ��:" << " " << i << endl;
//				msgRecvList.push_back(i);
//				my_mutex.unlock();
//			}
//			else
//			{
//				cout << "û�õ�������Ϣ100����" << endl;
//				chrono::milliseconds dura(100);
//				this_thread::sleep_for(dura);
//			}
//		}
//	}
//	void outMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "outMsg����ִ�У�";
//			lock_guard<timed_mutex> myguard(my_mutex);
//			chrono::milliseconds dura(100000000);
//			this_thread::sleep_for(dura);
//			if (!msgRecvList.empty())
//			{
//				int command = msgRecvList.front();
//				cout << "��Ҫ�Ƴ���Ԫ��" << command << endl;
//
//				msgRecvList.pop_front();
//			}
//			else
//			{
//				cout << "����Ϊ��" << i << endl;
//			}
//
//		}
//	}
//	private:
//		list<int> msgRecvList;
//		timed_mutex my_mutex;
//	};
//int  main()
//{
//	A  a;
//	thread myInObj(&A::inMsgList, ref(a));
//	thread myOutObj(&A::outMsgList, &a);
//	myInObj.join();
//	myOutObj.join();
//}




//
//
//class A
//{
//public:
//	void inMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "inMsgList����ִ�У�" << endl;
//			chrono::milliseconds dura(100);
//			if (my_mutex.try_lock_until(chrono::steady_clock::now()+dura)) //��ǰʱ�����100
//			{
//				cout << "����һ��Ԫ��:" << " " << i << endl;
//				msgRecvList.push_back(i);
//				my_mutex.unlock();
//			}
//			else
//			{
//				cout << "û�õ�������Ϣ100����" << endl;
//				chrono::milliseconds dura(100);
//				this_thread::sleep_for(dura);
//			}
//		}
//	}
//	void outMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "outMsg����ִ�У�";
//			lock_guard<timed_mutex> myguard(my_mutex);
//			chrono::milliseconds dura(100000000);
//			this_thread::sleep_for(dura);
//			if (!msgRecvList.empty())
//			{
//				int command = msgRecvList.front();
//				cout << "��Ҫ�Ƴ���Ԫ��" << command << endl;
//
//				msgRecvList.pop_front();
//			}
//			else
//			{
//				cout << "����Ϊ��" << i << endl;
//			}
//
//		}
//	}
//private:
//	list<int> msgRecvList;
//	timed_mutex my_mutex;
//};
//int  main()
//{
//	A  a;
//	thread myInObj(&A::inMsgList, ref(a));
//	thread myOutObj(&A::outMsgList, &a);
//	myInObj.join();
//	myOutObj.join();
//}
//















































  