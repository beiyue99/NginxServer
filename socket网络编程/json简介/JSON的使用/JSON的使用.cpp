#include<iostream>
using namespace std;
#include"json.hpp"
using json = nlohmann::json;
#include<vector>
#include<map>

void test1()   //json�����л�
{
#if 0
	json js;
	// �������
	js["id"] = { 1,2,3,4,5 };
	// ���key-value
	js["name"] = "zhang san";
	// ��Ӷ���
	js["msg"]["zhang san"] = "hello world";
	js["msg"]["liu shuo"] = "hello china";
	// �����ͬ���������һ��������������
	//js["msg"] = { {"zhang san", "hello world"}, {"liu shuo", "hello china"} };
	cout << js << endl;
	//js��������л����:
	//{"id":[1,2,3,4,5],"msg":{"liu shuo":"hello china","zhang san":"hello world"},"name":"zhang san"}
#else
	json js;
	//ֱ�����л�һ��vector����
	vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(5);
	js["list"] = vec;
	//cout << js << endl;    //{"list":[1,2,5]}
	string sendBuf  =  js.dump();  // ��JSON ����ת��Ϊһ���ɹ��鿴���ַ�����ʽ
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
	string sendBuf  = js.dump();  //��json�������л�Ϊjson�ַ���
	return sendBuf;
}
string func2()
{
#if 1
	json js;
	// �������
	js["id"] = { 1,2,3,4,5 };
	// ���key-value
	js["name"] = "zhang san";
	// ��Ӷ���
	js["msg"]["zhang san"] = "hello world";
	js["msg"]["liu shuo"] = "hello china";
	return js.dump();
#else
	json js;
	//ֱ�����л�һ��vector����
	vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	vec.push_back(5);
	js["list"] = vec;
	string sendBuf = js.dump();
	return sendBuf;
#endif
}
void test2() //json�ķ����л�
{
	string recvBuf = func1();  //����func1�������緢���������л�������
	json jsbuf = json::parse(recvBuf);//���ݵķ����л�json�ַ��������л�Ϊjson���ݶ���(��������,�������)
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

	auto arr = jsbuf["list"];        //vector����
	cout << arr[2] << endl;             //5

#endif
}
int main()
{
	//test1();    //json�����л�
	//test2();      //json�ķ����л�
	test3();      //json�ķ����л�++
}

