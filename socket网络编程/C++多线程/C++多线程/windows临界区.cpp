//#define _CRT_SECURE_NO_WARNINGS 1
//#include<iostream>
//using namespace std;
//
//////windows�ٽ���
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
//			cout << "inMsgList����ִ�У�" << "����һ��Ԫ��:" << " " << i << endl;
//		
//#ifdef __WINDOWSJQ_
//			EnterCriticalSection(&my_winsec);   //���������������Σ���ҲҪ�뿪���Σ������൱����û����
//			msgRecvList.push_back(i);           //mutex������������ס���Σ�ֱ�ӱ������ǵݹ���
//			LeaveCriticalSection(&my_winsec);
//#else  
//			my_mutex.lock();
//			msgRecvList.push_back(i);
//			my_mutex.unlock();
//#endif
//			cout << "endif���յ㣬�Ժ�Ĵ��붼����ִ��Ŷ~" << endl;
//		}
//	}
//	void outMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "outMsg����ִ�У�";
//#ifdef __WINDOWSJQ_
//			EnterCriticalSection(&my_winsec);
//			if (!msgRecvList.empty())
//			{
//				int command = msgRecvList.front();
//				cout << "��Ҫ�Ƴ���Ԫ��" << command << endl;
//				msgRecvList.pop_front();
//				LeaveCriticalSection(&my_winsec);
//			}
//			else
//			{
//				cout << "����Ϊ��" << i << endl;
//				LeaveCriticalSection(&my_winsec);
//			}
//#else       
//			lock_guard<mutex> myguard(my_mutex);
//			if (!msgRecvList.empty())
//			{
//				int command = msgRecvList.front();
//				cout << "��Ҫ�Ƴ���Ԫ��" << command << endl;
//				msgRecvList.pop_front();
//			}
//			else
//			{
//				cout << "����Ϊ��" << i << endl;
//			}
//#endif
//		}
//	}
//	A()
//	{
//#ifdef __WINDOWSJQ_
//		InitializeCriticalSection(&my_winsec);//��ʼ���ٽ���
//
//#endif
//	}
//private:
//	list<int > msgRecvList;
//	mutex my_mutex;
//#ifdef __WINDOWSJQ_
//	CRITICAL_SECTION my_winsec; //windows�е��ٽ���
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




