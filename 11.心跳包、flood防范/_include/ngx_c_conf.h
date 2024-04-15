
#ifndef __NGX_CONF_H__
#define __NGX_CONF_H__

#include <vector>
#include<mutex>
#include "ngx_global.h"  //一些全局/通用定义

class CConfig
{
private:
	CConfig();
	CConfig(const CConfig&) = delete; // 禁用拷贝构造
	CConfig& operator=(const CConfig&) = delete; // 禁用赋值运算符
	static CConfig *m_instance;
	static std::mutex m_mutex;
public:	
	~CConfig();
	static CConfig* GetInstance()
	{
		if (m_instance == nullptr)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if (m_instance == nullptr)
			{
				m_instance = new CConfig();
				static CGarhuishou cl;
			}
		}
		return m_instance;
	}


	bool Load(const char* pconfName); //装载配置文件
	const char* GetString(const char* p_itemname);
	int  GetIntDefault(const char* p_itemname, const int def);
	std::vector<LPCConfItem> m_ConfigItemList; //存储配置信息的列表

private:
	class CGarhuishou  //类中套类，用于释放对象
	{
	public:				
		~CGarhuishou()
		{
			if (CConfig::m_instance)
			{						
				delete CConfig::m_instance;				
				CConfig::m_instance = NULL;				
			}
		}
	};
};

#endif
