#include<iostream>
using namespace std;
#include<thread>












//如果类内有有参构造函数，则可以把参数传入，编译器会根据传入参数构造出一个类
//class A
//{
//public:
//	int m_i;
//	A(int a) :m_i(a) { cout << "有参构造！" << endl; }
//	~A() { cout << "析构!" << endl; }
//};
//
//void fun(const   A& a1)
//{
//	cout << &a1;
//	cout << endl << a1.m_i;
//}
//int main()
//{
//	int a = 12;
//	fun(a);
//}










//
//class A
//{
//public:
//	int m_i;
//	A(int a) :m_i(a)
//	{
//		cout << "有参构造函数执行！" << this << "  " << "threadid=" << this_thread::get_id() << endl;
//	}
//	A(const A& a) :m_i(a.m_i)
//	{
//		cout << "拷贝构造函数执行!" << this << "  " << "threadid=" << this_thread::get_id() << endl;
//	}
//	~A()
//	{
//		cout << "析构函数执行！" << this << "  " << "thradid=" << this_thread::get_id() << endl;
//	}
//	void thread_work(int num)
//	{
//		cout << "子线程work函数执行！" << this << "  " << "threadid=" << this_thread::get_id() << endl;
//	}
//	void operator()(int num)
//	{
//		cout << "子线程()执行" << this << "threadid=" << this_thread::get_id() << endl;
//	}
//};
//
//int main()
//{
//	A a(10);
//	thread myobj(&A::thread_work, ref(a), 15);       //若不传递引用 会拷贝新对象     
//	myobj.join();       // 给类内重载个可调用对象，此处传递俩参数，也会起到一样的效果。唯一区别是只能用ref,不可以使用&
//	//+++++++++++++++++++++++++++++++
//	//A a(10);
//	//thread myobj(ref(a), 15);      //调用重载函数
//	//myobj.join();
//}









//#include<vector>
//void print(int i)
//{
//	cout << "我的线程开始执行！" <<i<< endl;
//	cout << "我的线程执行完毕！" << endl;
//}
//int main()
//{
//	vector<thread> mythreads;
//	for (int i = 0; i < 10; i++)
//	{
//		mythreads.push_back(thread(print, i)); //十个线程开始执行，但是顺序不确定
//	}
//	for (auto it = mythreads.begin(); it != mythreads.end(); it++)
//	{
//		it->join();
//	}
//	cout << "i love china!" << endl;
//}





//
//#include<list>
//#include<mutex>
//
//class A
//{
//public:
//	void inMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "inMsgList函数执行！" << "插入一个元素:" << " " << i << endl;
//			my_mutex.lock();
//			msgRecvList.push_back(i);
//			my_mutex.unlock();
//		}
//	}
//	void outMsgList()
//	{
//		for (int i = 0; i < 100000; i++)
//		{
//			cout << "outMsg函数执行！";
//			//my_mutex.lock();
//			lock_guard<mutex> myguard(my_mutex);
//			if (!msgRecvList.empty())
//			{
//				int command = msgRecvList.front();
//				cout << "将要移除的元素" << command << endl;
//				msgRecvList.pop_front();
//				//my_mutex.unlock();
//			}
//			else
//			{
//				cout << "容器为空" << i << endl;
//				//my_mutex.unlock();
//			}
//		}
//	}
//private:
//	list<int > msgRecvList;
//	mutex my_mutex;
//};
//int  main()
//{
//	A  a;
//	thread myInObj(&A::inMsgList, ref(a));
//	thread myOutObj(&A::outMsgList, &a);
//	myInObj.join();
//	myOutObj.join();
//}








//#include <iostream>
//#include <vector>
//#include <functional>
//
//struct Foo {
//    int x;
//};
//
//int main() {
//    Foo a{ 10 };
//    Foo b{ 20 };
//
//    std::vector<std::reference_wrapper<Foo>> vec; 
//    // 成功：可以在vector中存放引用包装器  ,如果Foo的话，那么放入的是a,b的拷贝
//    vec.push_back(a);
//    vec.push_back(b);
//
//    for (auto& fooRef : vec) {
//        fooRef.get().x += 5;  // 修改vector中的Foo对象
//    }
//
//    std::cout << a.x << ", " << b.x << "\n";  // 输出：15, 25，显示原始对象a和b被修改了
//
//    return 0;
//}



//#include <iostream>
//#include <vector>
//#include <functional>
//
//struct Foo {
//    int x;
//};
//
//int main() {
//    Foo a{ 10 };
//    Foo b{ 20 };
//
//    std::vector<Foo&> vec; // 错误：不能在vector中存放引用
//    vec.push_back(a);
//    vec.push_back(b);
//
//    return 0;
//}
