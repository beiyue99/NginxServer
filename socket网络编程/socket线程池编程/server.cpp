#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include<pthread.h>
#include"thread.h"
#define PORT 8080
#define BUFFER_SIZE 1024

const char* message = "Hello My Client!";
char buffer[BUFFER_SIZE] ;
int server_fd,  valread;
int addrlen = sizeof(struct sockaddr_in);
//信息结构体
struct SockInfo
{
    struct sockaddr_in addr;
    int fd;
};
typedef struct PoolInfo
{
    ThreadPool* p;
    int fd;
}PoolInfo;

void working(void* arg);
void acceptConn(void* arg);

int main()
{
    int opt = 1;
    struct sockaddr_in address;

    // 创建服务器套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器套接字选项
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // 绑定服务器地址和端口
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听连接请求
    if (listen(server_fd, 5) == -1)
    {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
   
    //创建线程池
    ThreadPool* pool = threadPoolCreate(3, 8, 100);
    PoolInfo* info=(PoolInfo*)malloc(sizeof(PoolInfo));
    info->p = pool;
    info->fd = server_fd;
    threadPoolAdd(pool, acceptConn, info);
    pthread_exit(NULL);
    return 0;
}
void acceptConn(void* arg)
{
    PoolInfo* poolInfo=(PoolInfo*)arg;
    // 阻塞并等待客户端的连接
    while (1)
    {
        struct SockInfo* pinfo=(struct SockInfo*)malloc(sizeof(struct SockInfo));
        if ((pinfo->fd = accept(poolInfo->fd, (struct sockaddr*)&pinfo->addr, (socklen_t*)&addrlen)) == -1)
        {
            perror("accept failed");
            //break; //跳出循环
            continue; // 继续等待下一个连接
        }
        //添加通信的任务
        threadPoolAdd(poolInfo->p, working, pinfo);

    }
    close(poolInfo->fd);
}




void working(void* arg)
{
    struct SockInfo* pinfo = (struct SockInfo*)arg;
    if (pinfo == NULL)
    {
        fprintf(stderr, "Invalid pointer\n");
        return ;
    }
    //连接成功，打印客户端的IP和端口信息
    char ip[32];
    printf("Client IP: %s,Port: %d\n", inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, ip, sizeof(ip)), ntohs(pinfo->addr.sin_port));
    // 循环接收和发送消息
    while (1)
    {
        // 从客户端读取数据
        valread = read(pinfo->fd, buffer, BUFFER_SIZE);
        if (valread == -1) {
            perror("read failed");
            //exit(EXIT_FAILURE);
            pthread_exit(NULL);
        }
        else if (valread == 0) {
            // 客户端断开连接
            printf("Client disconnected\n");
            pthread_exit(NULL);
        }
        // 客户端主动关闭连接：当客户端调用close函数关闭套接字时，服务器端的read函数将返回0，表示连接已经结束。
        // 接收缓冲区为空：当客户端发送了FIN（结束）包后，服务器端读取缓冲区中已经接收到的数据后，再次调用read函数时，由于接收缓冲区已经为空，read函数将返回0。
        printf("Client: %s\n", buffer);

        // 判断通信是否结束
        if (strcmp(buffer, "quit") == 0) {
            break;  // 结束循环
        }

        // 向客户端发送消息
        if (send(pinfo->fd, message, strlen(message), 0) == -1) {
            perror("send failed");
            //exit(EXIT_FAILURE);
            //pthread_exit(NULL);
            break; // 跳出循环，处理客户端断开连接的情况
        }
        // printf("第%d次Message sent to the client succeesfully!\n",count++);

         // 清空缓冲区
        memset(buffer, 0, BUFFER_SIZE);
    }

    // 关闭套接字
    close(pinfo->fd);
}
