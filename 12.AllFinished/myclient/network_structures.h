#ifndef NETWORK_STRUCTURES_H
#define NETWORK_STRUCTURES_H

#include <cstring> // for strcpy

#pragma pack (1) // 对齐方式,1字节对齐

typedef struct _COMM_PKG_HEADER {
    unsigned short pkgLen;    // 报文总长度【包头+包体】--2字节
    unsigned short msgCode;   // 消息类型代码--2字节
    int crc32;     // CRC32校验--4字节
} COMM_PKG_HEADER, *LPCOMM_PKG_HEADER;

typedef struct _STRUCT_REGISTER {
    int iType;          // 类型
    char username[56];   // 用户名
    char password[40];   // 密码
} STRUCT_REGISTER, *LPSTRUCT_REGISTER;

typedef struct _STRUCT_LOGIN {
    char username[56];   // 用户名
    char password[40];   // 密码
} STRUCT_LOGIN, *LPSTRUCT_LOGIN;

#pragma pack() // 取消指定对齐，恢复缺省对齐

extern int g_iLenPkgHeader;

#endif // NETWORK_STRUCTURES_H
