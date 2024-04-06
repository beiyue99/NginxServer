#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
using namespace std;
//shared_future     该类的成员函数get可以多次调用
//#include<future>
//int mythread(int mypar)
//{
//	cout << mypar << endl;
//	cout << "mythread() Start!" << "threadid=" << this_thread::get_id() << endl;
//	chrono::milliseconds dura(2000);
//	this_thread::sleep_for(dura);
//	cout << "mythread() End!" << "threadid=" << this_thread::get_id() << endl;
//	return 5;
//}
//void mythread2(shared_future<int>& tmpf)
//{
//	auto result = tmpf.get(); //不能多次get是因为里面采用的是移动语义
//	cout << "mythread2 result:" << result << endl;
//}
//int main()
//{
//	cout << "main threadid=" << this_thread::get_id() << endl;
//	packaged_task<int(int)>mypt(mythread);
//	thread t1(ref(mypt), 1); 
//	t1.join();
//	future<int> fu1 = mypt.get_future();      //mypt也只能调用一次get_furure
//	bool iscanget = fu1.valid();
//	cout << iscanget << " ture" << endl;
//	//shared_future<int>fu2(move(fu1));
//	 shared_future<int>fu2(fu1.share());//或者
//	 iscanget = fu1.valid();
//	cout << iscanget << " false" << endl;
//	auto m1 = fu2.get();
//	auto m2 = fu2.get();
//	thread t2(mythread2, ref(fu2));
//	t2.join();
//	cout << "i love china!" << endl;
//}