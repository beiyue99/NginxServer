#define _CRT_SECURE_NO_WARNINGS 1

#include<iostream>
using namespace std;


//async : �����߳̽������ٽ������߳�


////async,future������̨���񲢷���ֵ��ϣ���̷߳���һ�����
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
//	future<int>result = async(launch::deferred,mythread);//�����̲߳���ʼִ�У����mythread�в�������ô���ڵڶ���λ�ô������
//	                          //����deferred�󣬲��ᴴ�����̣߳����ж����������߳�����ִ�У�
//							  // ����defrred��ʵ�����ǵ���luach::async|defrred����ʱ���ܴ������߳�Ҳ���ܲ�����
//	cout << "cotinue......!" << endl;
//	int def = 0;
//	cout << result.get() << endl;//��������ȴ�mythreadִ���꣬��ȡ����ֵ
//	//result.wait();//��������ȴ�mythreadִ���꣬���᷵�ؽ��
//	//�̱߳��ӳ�ִ�У��ӳٵ�get��wait�������ã���������ã��߳̾Ͳ���ִ�У��������
//	cout << "i love china!" << endl;
//	return 0;
//}

//thread�����̣߳������Դ���ţ��ͻᴴ��ʧ�ܣ����쳣��ϵͳ��������asyncĬ�ϲ�ָ������ʱ�������,�����ᴴ�����̣߳�
// ����˭������get��wait���ͻ�ֱ�������߳����С�����ֵ��Ҫ��ȫ�ֱ������������ֶ�������
//async�����߳��п��ܳɹ�Ҳ�п���ʧ�ܣ�������future���շ���ֵ









//��������������������������������





//future_status   �ж�async������û�д������߳�
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
//	future_status status = result.wait_for(chrono::seconds(6));  //wait(0s) ��0��  Ҳ����0min
//	if (status == future_status::timeout)
//	{
//		cout << "�̻߳�ûִ���꣡" << endl;
//		cout << result.get();    //���߳�ִ������õ����
//	}
//	else if (status==future_status::ready)
//	{
//		cout << "�̳߳ɹ�ִ����ϣ�" << endl;
//		cout << result.get();
//	}
//	else if (status == future_status::deferred)
//	{
//		cout << "�̱߳��ӳ�ִ�У�" << endl;
//		cout << result.get();
//	}
//	return 0;
//}
