#ifndef __NGX_MEMORY_H__
#define __NGX_MEMORY_H__
#include <cstddef>
#include "ngx_c_lockmutex.h"
#include<mutex>
#include<cstring>
#include<memory>
//内存相关类
class CMemory 
{
public:	
	CMemory() = default;
	~CMemory() = default;
	// 删除拷贝构造函数和赋值运算符
	CMemory(const CMemory&) = delete;
	CMemory& operator=(const CMemory&) = delete;
	// 获取单例实例
	static CMemory& GetInstance();
	// 分配与释放内存
	void* AllocMemory(std::size_t memCount, bool ifmemset = false);
	void FreeMemory(void* ptr) noexcept;
private:
	static std::unique_ptr<CMemory> m_instance;
	static std::once_flag m_onceFlag;
};
#endif
