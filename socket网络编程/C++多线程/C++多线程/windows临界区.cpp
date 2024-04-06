//#define _CRT_SECURE_NO_WARNINGS 1
//#include<iostream>
//using namespace std;
//
//////windows临界区
//#include<list>
//#include<mutex>
//#include<windows.h>
//
//
//#define __WINDOWSJQ_
//class A
//{
//public:
//	void inMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "inMsgList函数执行！" << "插入一个元素:" << " " << i << endl;
//		
//#ifdef __WINDOWSJQ_
//			EnterCriticalSection(&my_winsec);   //可以连续进入两次，但也要离开两次，否则相当于锁没撒开
//			msgRecvList.push_back(i);           //mutex不允许连续锁住两次，直接报错，除非递归锁
//			LeaveCriticalSection(&my_winsec);
//#else  
//			my_mutex.lock();
//			msgRecvList.push_back(i);
//			my_mutex.unlock();
//#endif
//			cout << "endif是终点，以后的代码都可以执行哦~" << endl;
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




