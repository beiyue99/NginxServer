#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
using namespace std;
#include<thread>
#include<vector>
#include<future>


#include<list>
#include<mutex>
#include<windows.h>


//  time_mutex   在mutex基础上多了两个成员函数
//  try_lock_for() 等待一定时间，再往下走
//  try_lock_until 等到未来的时间点，如果到这个时间之前拿到了锁就往下走，时点到了，没拿到锁也会往下走

//
//class A
//{
//public:
//	void inMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "inMsgList函数执行！" << endl;
//			chrono::milliseconds dura(100);
//			if (my_mutex.try_lock_for(dura))
//			{
//				cout << "插入一个元素:" << " " << i << endl;
//				msgRecvList.push_back(i);
//				my_mutex.unlock();
//			}
//			else
//			{
//				cout << "没拿到锁，休息100毫秒" << endl;
//				chrono::milliseconds dura(100);
//				this_thread::sleep_for(dura);
//			}
//		}
//	}
//	void outMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "outMsg函数执行！";
//			lock_guard<timed_mutex> myguard(my_mutex);
//			chrono::milliseconds dura(100000000);
//			this_thread::sleep_for(dura);
//			if (!msgRecvList.empty())
//			{
//				int command = msgRecvList.front();
//				cout << "将要移除的元素" << command << endl;
//
//				msgRecvList.pop_front();
//			}
//			else
//			{
//				cout << "容器为空" << i << endl;
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
//			cout << "inMsgList函数执行！" << endl;
//			chrono::milliseconds dura(100);
//			if (my_mutex.try_lock_until(chrono::steady_clock::now()+dura)) //当前时间加上100
//			{
//				cout << "插入一个元素:" << " " << i << endl;
//				msgRecvList.push_back(i);
//				my_mutex.unlock();
//			}
//			else
//			{
//				cout << "没拿到锁，休息100毫秒" << endl;
//				chrono::milliseconds dura(100);
//				this_thread::sleep_for(dura);
//			}
//		}
//	}
//	void outMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "outMsg函数执行！";
//			lock_guard<timed_mutex> myguard(my_mutex);
//			chrono::milliseconds dura(100000000);
//			this_thread::sleep_for(dura);
//			if (!msgRecvList.empty())
//			{
//				int command = msgRecvList.front();
//				cout << "将要移除的元素" << command << endl;
//
//				msgRecvList.pop_front();
//			}
//			else
//			{
//				cout << "容器为空" << i << endl;
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















































  