#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
using namespace std;

//promise
//#include<future>
//void mythread(promise<int>& tmpp, int calc)
//{
//	cout << "�߳̿�ʼ��"<< endl;
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
//	promise<int> myprom;  //����һ��promise���󣬱����ֵ������Ϊint
//	thread t1(mythread, ref(myprom), 180);
//	t1.join();
//	future<int> fu1 = myprom.get_future();
//	//auto result = fu1.get();   
//	//�ò��������һֱ���߳̽������ؽ���� ������ȻҪjoin����detach����Ȼ���쳣
//	//getֻ�ܵ���һ�Σ��߳�2��ʹ����fu1��Ϊ���õ���get��������������ע�͵�
//	//cout << "result=" << result << endl;
//	thread t2(mythread2, ref(fu1));
//	t2.join();
//	cout << "i love china!" << endl;
//}
