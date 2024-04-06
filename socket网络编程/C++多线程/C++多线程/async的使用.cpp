#define _CRT_SECURE_NO_WARNINGS 1

#include<iostream>
using namespace std;


//async : 等子线程结束，再结束主线程


////async,future创建后台任务并返回值，希望线程返回一个结果
#include<future>
//int mythread()
//{
//	cout << "mythread() Start!" << "threadid=" << this_thread::get_id() << endl;
//	chrono::milliseconds dura(5000);
//	this_thread::sleep_for(dura);
//	cout << "mythread() End!" << "threadid=" << this_thread::get_id() << endl;
//	return 5;
//}
//int main()
//{
//	cout << "main threadid=" << this_thread::get_id() << endl;
//	future<int>result = async(launch::deferred,mythread);//创建线程并开始执行，如果mythread有参数，那么就在第二个位置传入参数
//	                          //加入deferred后，不会创建子线程，所有东西都在主线程里面执行！
//							  // 不加defrred，实际上是调用luach::async|defrred，此时可能创建新线程也可能不创建
//	cout << "cotinue......!" << endl;
//	int def = 0;
//	cout << result.get() << endl;//卡在这里等待mythread执行完，获取返回值
//	//result.wait();//卡在这里等待mythread执行完，不会返回结果
//	//线程被延迟执行，延迟到get或wait函数调用，如果不调用，线程就不会执行，程序结束
//	cout << "i love china!" << endl;
//	return 0;
//}

//thread创建线程，如果资源紧张，就会创建失败，报异常，系统崩溃，而async默认不指定参数时不会崩溃,但不会创建新线程，
// 将来谁调用了get或wait，就会直接在主线程运行。返回值需要用全局变量或者其他手段来接收
//async创建线程有可能成功也有可能失败，可以用future接收返回值









//。。。。。。。。。。。。。。。。





//future_status   判断async到底有没有创建新线程
//#include<future>
//int mythread()
//{
//	cout << "mythread() Start!" << "threadid=" << this_thread::get_id() << endl;
//	chrono::milliseconds dura(5000);
//	this_thread::sleep_for(dura);
//	cout << "mythread() End!" << "threadid=" << this_thread::get_id() << endl;
//	return 5;
//}
//int main()
//{
//	cout << "main threadid=" << this_thread::get_id() << endl;
//	future<int>result = async( mythread);
//	cout << "cotinue......!" << endl;
//	//cout << result.get() << endl;
//	future_status status = result.wait_for(chrono::seconds(6));  //wait(0s) 等0秒  也可以0min
//	if (status == future_status::timeout)
//	{
//		cout << "线程还没执行完！" << endl;
//		cout << result.get();    //等线程执行完后拿到结果
//	}
//	else if (status==future_status::ready)
//	{
//		cout << "线程成功执行完毕！" << endl;
//		cout << result.get();
//	}
//	else if (status == future_status::deferred)
//	{
//		cout << "线程被延迟执行！" << endl;
//		cout << result.get();
//	}
//	return 0;
//}
