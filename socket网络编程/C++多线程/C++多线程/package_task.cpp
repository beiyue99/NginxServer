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
//	//使用std::ref可以在模板传参的时候传入引用，thread的方法传递引用的时候，也要用ref，否则无法传递。
//	thread t1(ref(mypt), 1);  //线程直接执行，1代表线程函数的参数
//	t1.join();
//	future<int> result = mypt.get_future();
//	cout << result.get() << endl;
//	cout << "i love china!" << endl;
//}
//





#include<future>


//packaged_task的直接调用   不创建新线程
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
//packaged_task存在容器里调用
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
//	v1.push_back(move(mypt));         //必须用move，可能是独占对象，不允许拷贝
//	packaged_task<int(int)>mypt2;
//	auto it = v1.begin();
//	mypt2 = move(*it);
//	v1.erase(it); //it迭代器已被删除，后续不能再使用it迭代器
//	mypt2(200);
//	future<int> result = mypt2.get_future();
//	cout << result.get() << endl;
//}
