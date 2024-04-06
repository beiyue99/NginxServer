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
//		//atomic支持 ++ -- += -=  这里改为my_count=my_count+1就会出错
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
//	//原子对象内不含拷贝构造函数，只能用load函数 store函数
//	A()
//	{
//		atomic<int>atm2 = (atm.load());
//		atm2.store(20);
//		atm = 20;//不确定是不是原子操作
//	}
//	void inMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			countt++;
//			atm += 1; //原子操作不会被打断
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