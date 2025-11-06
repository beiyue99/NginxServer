#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <iostream>
#include "ngx_c_memory.h"

// 静态成员初始化
std::unique_ptr<CMemory> CMemory::m_instance;
std::once_flag CMemory::m_onceFlag;

// 获取单例实例
//CMemory& CMemory::GetInstance() {
//    std::call_once(m_onceFlag, []() {
//        m_instance = std::make_unique<CMemory>();
//        });
//    return *m_instance;
//}
CMemory& CMemory::GetInstance() {
    static CMemory instance;
    return instance;
}

// 分配内存
void* CMemory::AllocMemory(std::size_t memCount, bool ifmemset) {
    try {
        auto buffer = std::make_unique<char[]>(memCount);
        if (ifmemset) {
            std::memset(buffer.get(), 0, memCount);
        }
        return buffer.release(); // 返回裸指针，交由调用方管理
    }
    catch (const std::bad_alloc&) {
        std::cerr << "CMemory::AllocMemory() - 内存分配失败！\n";
        std::terminate(); // 或者 throw; 取决于你的项目风格
    }
}

// 释放内存
void CMemory::FreeMemory(void* ptr) noexcept {
    delete[] static_cast<char*>(ptr);
}
