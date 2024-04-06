#define _CRT_SECURE_NO_WARNINGS 1
//死锁
#include<iostream>
using namespace std;
#include<mutex>
#include<chrono>
#include<list>
#include <thread>

class A
{
public:
	void inMsgList()
	{
		for (int i = 0; i < 50000; i++)
		{
			unique_lock<mutex>sbguard1(my_mutex1, try_to_lock);       //在没锁的前提下使用try_to_lock
			if (sbguard1.owns_lock())//拿到了锁
			{
				cout << "inMsgList函数执行！" << "插入一个元素:" << " " << i << endl;
				msgRecvList.push_back(i);
			}
			else
			{
				cout << "inMsgList执行,但没拿到锁!" << endl;
			}
			//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	/*		cout << "inMsgList函数执行！" << "插入一个元素:" << " " << i << endl;
			lock(my_mutex1, my_mutex2);
			msgRecvList.push_back(i);
			my_mutex2.unlock();
			my_mutex1.unlock();*/
		}
	}
	void outMsgList()
	{
		for (int i = 0; i < 50000; i++)
		{
			cout << "outMsg函数执行！";
			//lock_guard<mutex> myguard2(my_mutex2);
			lock_guard<mutex> myguard1(my_mutex1);
			chrono::seconds dura(1);
			this_thread::sleep_for(dura);
			if (!msgRecvList.empty())
			{
				int command = msgRecvList.front();
				cout << "将要移除的元素" << command << endl;
				msgRecvList.pop_front();
			}
			else
			{
				cout << "容器为空" << i << endl;
			}
		/*	my_mutex1.unlock();
			my_mutex2.unlock();*/
		}
	}
private:
	list<int > msgRecvList;
	mutex my_mutex1;
	mutex my_mutex2;
};
int  main()
{
	A  a;
	thread myInObj(&A::inMsgList, &a);
	thread myOutObj(&A::outMsgList, &a);
	myInObj.join();
	myOutObj.join();
}






//lock(my_mutex2, my_mutex1);    
//lock_guard<mutex> myguard1(my_mutex2,adopt_lock);
//lock_guard<mutex> myguard2(my_mutex1,adopt_lock);    
//当你已经手动锁定一个互斥量，并且希望把它的所有权转移给
//  std::lock_guard 或 std::unique_lock 对象时，你可以使用 std::adopt_lock