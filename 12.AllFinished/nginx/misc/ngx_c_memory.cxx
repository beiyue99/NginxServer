#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <iostream>
#include "ngx_c_memory.h"



CMemory& CMemory::GetInstance() {
    static CMemory instance;
    return instance;
}


CMemory::BufferPtr CMemory::AllocBuffer(std::size_t memCount, bool ifmemset)
{
    auto buffer = std::make_unique<char[]>(memCount);
    if (ifmemset)
    {
        std::memset(buffer.get(), 0, memCount);
    }
    return buffer;
}