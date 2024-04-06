#include<iostream>
using namespace std;
#include<thread>
#include<mutex>
#include<list>
#include<json/json.h>
#include<fstream>
using namespace Json;




/*
[
	"1uffy", 19, 170, fa1se,
	["ace", "sabo"],
	{ "sex":"man","girlfriend" : "Hancock"}
]
*/

void writeJson()
{
	Value root;
	root.append("luffy");
	root.append("19");
	root.append("170");
	root.append("false");

	Value subArray;
	subArray.append("ace");
	subArray.append("sabo");
	root.append(subArray);

	Value obj;
	obj["sex"] = "man";
	obj["girlfirend"] = "Hancock";
	root.append(obj);
#if 0

	string json = root.toStyledString(); //有格式
#else
	FastWriter w;
	string json = w.write(root);         //无格式
#endif


	ofstream ofs("test.json");   //文件名为test.json
	ofs << json;
	ofs.close();
}

void readJson()
{
	ifstream ifs("test.ison");
	Reader rd;
	Value root;
	rd.parse(ifs, root);

	if (root.isArray())
	{
		for (int i = 0; i < root.size(); ++i)
		{
			Value item = root[i];
			if (item.isInt())
			{
				cout << item.asInt() << ",";
			}
			else if (item.isString())
			{
				cout << item.asString() << ",";
			}
			else if (item.isBool())
			{
				cout << item.asBool() << ",";
			}
			else if (item.isArray())
			{
				for (int j = 0; j < item.size(); ++j)
					cout << item[j].asString() << ", ";
			}
			else if (item.isObject())
			{
				Value::Members keys = item.getMemberNames();
				for (int k = 0; k < keys.size(); ++k)
				{
					cout << keys.at(k) << ":" << item[keys[k]] << ",";
				}
			}
			cout << endl;
		}
	}
	else
	{

	}
}


int main()
{
	readJson();
	writeJson();
}







