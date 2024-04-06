#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
using namespace std;

//promise
//#include<future>
//void mythread(promise<int>& tmpp, int calc)
//{
//	cout << "线程开始！"<< endl;
//	calc++;
//	calc *= 10;
//	chrono::seconds dura(3);
//	this_thread::sleep_for(dura);
//	int result = calc;
//	tmpp.set_value(result);
//}
//void mythread2(future<int>& tmpf)
//{
//	auto result = tmpf.get();
//	cout << "mythread2 result:" << result << endl;
//}
//int main()
//{
//	promise<int> myprom;  //声明一个promise对象，保存的值得类型为int
//	thread t1(mythread, ref(myprom), 180);
//	t1.join();
//	future<int> fu1 = myprom.get_future();
//	//auto result = fu1.get();   
//	//拿不到结果会一直等线程结束返回结果。 但是依然要join或者detach，不然报异常
//	//get只能调用一次，线程2中使用了fu1作为引用调用get函数，所以这里注释掉
//	//cout << "result=" << result << endl;
//	thread t2(mythread2, ref(fu1));
//	t2.join();
//	cout << "i love china!" << endl;
//}
