
#ifndef __NGX_MEMORY_H__
#define __NGX_MEMORY_H__

#include <stddef.h>  //NULL
#include<mutex>
#include "ngx_c_lockmutex.h"
//内存相关类
class CMemory 
{
private:
	CMemory() {}  //构造函数，因为要做成单例类，所以是私有的构造函数

	~CMemory(){};

private:
	static CMemory *m_instance;
	static std::mutex m_mutex;

public:	
	// 删除拷贝构造函数和赋值运算符
	CMemory(const CMemory&) = delete;
	CMemory& operator=(const CMemory&) = delete;
	static CMemory* GetInstance() //单例
	{			
		if(m_instance == NULL)
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			if(m_instance == NULL)
			{				
				m_instance = new CMemory(); 
				static CGarhuishou cl; 
			}
		}
		return m_instance;
	}	
	class CGarhuishou 
	{
	public:				
		~CGarhuishou()
		{
			if (CMemory::m_instance)
			{						
				delete CMemory::m_instance; 
				CMemory::m_instance = NULL;				
			}
		}
	};

public:
	void *AllocMemory(int memCount,bool ifmemset);
	void FreeMemory(void *point);
	
};

#endif
