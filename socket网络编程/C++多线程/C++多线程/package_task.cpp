#define _CRT_SECURE_NO_WARNINGS 1


//packaged_task
//int mythread(int mypar)
//{
//	cout << mypar << endl;
//	cout << "mythread() Start!" << "threadid=" << this_thread::get_id() << endl;
//	chrono::milliseconds dura(2000);
//	this_thread::sleep_for(dura);
//	cout << "mythread() End!" << "threadid=" << this_thread::get_id() << endl;
//	return 5;
//}
//int main()
//{
//	cout << "main threadid=" << this_thread::get_id() << endl;
//	packaged_task<int(int)>mypt(mythread);
//	//ʹ��std::ref������ģ�崫�ε�ʱ�������ã�thread�ķ����������õ�ʱ��ҲҪ��ref�������޷����ݡ�
//	thread t1(ref(mypt), 1);  //�߳�ֱ��ִ�У�1�����̺߳����Ĳ���
//	t1.join();
//	future<int> result = mypt.get_future();
//	cout << result.get() << endl;
//	cout << "i love china!" << endl;
//}
//





#include<future>


//packaged_task��ֱ�ӵ���   ���������߳�
//int mythread(int mypar)
//{
//	cout << mypar << endl;
//	cout << "mythread() Start!" << "threadid=" << this_thread::get_id() << endl;
//	chrono::milliseconds dura(2000);
//	this_thread::sleep_for(dura);
//	cout << "mythread() End!" << "threadid=" << this_thread::get_id() << endl;
//	return 5;
//}
//int main()
//{
//	cout << "main threadid=" << this_thread::get_id() << endl;
//	packaged_task<int(int)>mypt(mythread);
//	mypt(200);
//	future<int> result = mypt.get_future();
//	cout << result.get() << endl;
//}








#include<future>

#include<vector>
//packaged_task�������������
//int mythread(int mypar)
//{
//	cout << mypar << endl;
//	cout << "mythread() Start!" << "threadid=" << this_thread::get_id() << endl;
//	chrono::milliseconds dura(2000);
//	this_thread::sleep_for(dura);
//	cout << "mythread() End!" << "threadid=" << this_thread::get_id() << endl;
//	return 5;
//}
//int main()
//{
//	vector < packaged_task<int(int)>> v1;
//	cout << "main threadid=" << this_thread::get_id() << endl;
//	packaged_task<int(int)>mypt(mythread);
//	v1.push_back(move(mypt));         //������move�������Ƕ�ռ���󣬲�������
//	packaged_task<int(int)>mypt2;
//	auto it = v1.begin();
//	mypt2 = move(*it);
//	v1.erase(it); //it�������ѱ�ɾ��������������ʹ��it������
//	mypt2(200);
//	future<int> result = mypt2.get_future();
//	cout << result.get() << endl;
//}
