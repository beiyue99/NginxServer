#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
using namespace std;

//自动析构--- 仿造lock_guard
//#include<list>
//#include<mutex>
//#include<windows.h>
//
//
//#define __WINDOWSJQ_
//class CwinLock
//{
//public:
//	CwinLock(CRITICAL_SECTION* pCritemp)
//	{
//		m_pCritical = pCritemp;
//		EnterCriticalSection(m_pCritical);
//	}
//	~CwinLock()
//	{
//		LeaveCriticalSection(m_pCritical);
//	}
//private:
//	CRITICAL_SECTION* m_pCritical;
//};
//
//
//class A
//{
//public:
//	void inMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "inMsgList函数执行！" << "插入一个元素:" << " " << i << endl;
//#ifdef __WINDOWSJQ_
//			CwinLock wlock(&my_winsec);
//			CwinLock wlock2(&my_winsec);    //可以多次进入，而lock_guard不可以这样，否则报错
//			msgRecvList.push_back(i);
//#else  
//			my_mutex.lock();
//			msgRecvList.push_back(i);
//			my_mutex.unlock();
//#endif
//		}
//	}
//	void outMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "outMsg函数执行！";
//#ifdef __WINDOWSJQ_
//			EnterCriticalSection(&my_winsec);
//			if (!msgRecvList.empty())
//			{
//				int command = msgRecvList.front();
//				cout << "将要移除的元素" << command << endl;
//				msgRecvList.pop_front();
//				LeaveCriticalSection(&my_winsec);
//			}
//			else
//			{
//				cout << "容器为空" << i << endl;
//				LeaveCriticalSection(&my_winsec);
//			}
//#else       
//			lock_guard<mutex> myguard(my_mutex);
//			if (!msgRecvList.empty())
//			{
//				int command = msgRecvList.front();
//				cout << "将要移除的元素" << command << endl;
//				msgRecvList.pop_front();
//			}
//			else
//			{
//				cout << "容器为空" << i << endl;
//			}
//#endif
//		}
//	}
//	A()
//	{
//#ifdef __WINDOWSJQ_
//		InitializeCriticalSection(&my_winsec);//初始化临界区
//
//#endif
//	}
//private:
//	list<int > msgRecvList;
//	mutex my_mutex;
//#ifdef __WINDOWSJQ_
//	CRITICAL_SECTION my_winsec; //windows中的临界区
//#endif
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

