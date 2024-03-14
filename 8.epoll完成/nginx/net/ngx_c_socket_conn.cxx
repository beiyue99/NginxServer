
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>    //uintptr_t
#include <stdarg.h>    //va_start....
#include <unistd.h>    //STDERR_FILENO等
#include <sys/time.h>  //gettimeofday
#include <time.h>      //localtime_r
#include <fcntl.h>     //open
#include <errno.h>     //errno
//#include <sys/socket.h>
#include <sys/ioctl.h> //ioctl
#include <arpa/inet.h>

#include "ngx_c_conf.h"
#include "ngx_macro.h"
#include "ngx_global.h"
#include "ngx_func.h"
#include "ngx_c_socket.h"

lpngx_connection_t CSocekt::ngx_get_connection(int isock)
{
    lpngx_connection_t  c = m_pfree_connections; //空闲连接链表头
    if(c == NULL)
    {
        ngx_log_stderr(0,"CSocekt::ngx_get_connection()中空闲链表为空,这不应该!");
        return NULL;
    }

    m_pfree_connections = c->data;                       //指向连接池中下一个未用的节点
    m_free_connection_n--;                               //空闲连接少1
    
    uint64_t iCurrsequence = c->iCurrsequence;

    //(2)把以往有用的数据搞出来后，清空并给适当值
    memset(c,0,sizeof(ngx_connection_t));               
    c->fd = isock;                                       //套接字要保存起来，这东西具有唯一性    
    c->iCurrsequence=iCurrsequence;++c->iCurrsequence;  //每次取用该值都增加1
    return c;    
}

//归还参数c所代表的连接到到连接池中，注意参数类型是lpngx_connection_t
void CSocekt::ngx_free_connection(lpngx_connection_t c) 
{
    c->data = m_pfree_connections;      //回收的节点指向原来串起来的空闲链的链头
    ++c->iCurrsequence;                 //回收后，该值就增加1,以用于判断某些网络事件是否过期【一被释放就立即+1也是有必要的】
    m_pfree_connections = c;            //修改 原来的链头使链头指向新节点
    ++m_free_connection_n;              //空闲连接多1    
    return;
}

