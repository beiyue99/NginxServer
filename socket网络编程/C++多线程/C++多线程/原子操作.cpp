#define _CRT_SECURE_NO_WARNINGS 1


#include<iostream>
using namespace std;
#include<list>
#include<mutex>

//
//#include<mutex>
//int my_count;
//mutex m1;
//
//void mythread()
//{
//	for (int i = 0; i < 10000000; i++)
//	{
//		//m1.lock();
//		my_count++;
//		//m1.unlock();
//	}
//}
//int main()
//{
//	thread t1(mythread);
//	thread t2(mythread);
//	t1.join();
//	t2.join();
//	cout << my_count;
//}





//atomic

//atomic<int> my_count = 0;
//namespace ss
//{
//	int count = 0;
//}
//void mythread()
//{
//
//	for (int i = 0; i < 10000000; i++)
//	{
//		my_count++;
//		ss::count++;
//
//
//
//		//atomic֧�� ++ -- += -=  �����Ϊmy_count=my_count+1�ͻ����
//	}
//}
//int main()
//{
//	thread t1(mythread);
//	thread t2(mythread);
//	t1.join();
//	t2.join();
//	cout << my_count << endl;
//	cout << ss::count;
//}


//
//
//
//class A
//{
//public:
//	int countt;
//	atomic<int> atm;
//
//	//ԭ�Ӷ����ڲ����������캯����ֻ����load���� store����
//	A()
//	{
//		atomic<int>atm2 = (atm.load());
//		atm2.store(20);
//		atm = 20;//��ȷ���ǲ���ԭ�Ӳ���
//	}
//	void inMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			countt++;
//			atm += 1; //ԭ�Ӳ������ᱻ���
//		}
//	}
//	void outMsgList()
//	{
//		while (true)
//		{
//			cout << atm << "|||" ;
//			cout << countt << endl;
//		}
//	}
//private:
//	list<int > msgRecvList;
//};
//int  main()
//{
//	A  a;
//	
//	thread myInObj(&A::inMsgList, &a);
//	thread myInObj2(&A::inMsgList, &a);
//	thread myOutObj(&A::outMsgList, &a);
//	myInObj.join();
//	myInObj2.join();
//	myOutObj.join();
//}