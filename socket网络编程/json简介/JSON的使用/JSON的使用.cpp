#include<iostream>
using namespace std;
#include"json.hpp"
using json = nlohmann::json;
#include<vector>
#include<map>

void test1()   //json的序列化
{
#if 0
	json js;
	// 添加数组
	js["id"] = { 1,2,3,4,5 };
	// 添加key-value
	js["name"] = "zhang san";
	// 添加对象
	js["msg"]["zhang san"] = "hello world";
	js["msg"]["liu shuo"] = "hello china";
	// 上面等同于下面这句一次性添加数组对象
	//js["msg"] = { {"zhang san", "hello world"}, {"liu shuo", "hello china"} };
	cout << js << endl;
	//js对象的序列化结果:
	//{"id":[1,2,3,4,5],"msg":{"liu shuo":"hello china","zhang san":"hello world"},"name":"zhang san"}
#else
	json js;
	//直接序列化一个vector容器
	vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(5);
	js["list"] = vec;
	//cout << js << endl;    //{"list":[1,2,5]}
	string sendBuf  =  js.dump();  // 把JSON 对象转储为一个可供查看的字符串格式
	cout << sendBuf << endl;
#endif
}

string func1() 
{
	json js;
	js["msg_type"] = 2;
	js["from"] = "zhang san";
	js["to"] = "li si";
	js["msg"] = "hello,what are you doing now?";
	string sendBuf  = js.dump();  //把json对象序列化为json字符串
	return sendBuf;
}
string func2()
{
#if 1
	json js;
	// 添加数组
	js["id"] = { 1,2,3,4,5 };
	// 添加key-value
	js["name"] = "zhang san";
	// 添加对象
	js["msg"]["zhang san"] = "hello world";
	js["msg"]["liu shuo"] = "hello china";
	return js.dump();
#else
	json js;
	//直接序列化一个vector容器
	vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(5);
	js["list"] = vec;
	string sendBuf = js.dump();
	return sendBuf;
#endif
}
void test2() //json的反序列化
{
	string recvBuf = func1();  //假如func1就是网络发送来的序列化的数据
	json jsbuf = json::parse(recvBuf);//数据的反序列化json字符串反序列化为json数据对象(看作容器,方便访问)
	cout << jsbuf["msg_type"] << endl;
	cout << jsbuf["from"] << endl;
	cout << jsbuf["to"] << endl;
	cout << jsbuf["msg"] << endl;
}
void test3()
{
	string recvBuf = func2();  
	json jsbuf = json::parse(recvBuf);
#if 0
	cout << jsbuf["id"] << endl;      //[1,2,3,4,5]
	cout << jsbuf["name"] << endl;    //"zhang san"
	cout << jsbuf["msg"] << endl;     //{"liu shuo":"hello china","zhang san":"hello world"}
	auto arr = jsbuf["msg"];
	cout << arr["zhang san"] << endl; //"hello world"
	cout << arr["liu shuo"] << endl;  //"hello china"
#else
	cout << jsbuf << endl;	            //{"list": [1, 2, 5] }
	cout << jsbuf["list"] << endl;	    //[1, 2, 5]
	cout << jsbuf["list"][1] << endl;	//2

	auto arr = jsbuf["list"];        //vector类型
	cout << arr[2] << endl;             //5

#endif
}
int main()
{
	//test1();    //json的序列化
	//test2();      //json的反序列化
	test3();      //json的反序列化++
}

