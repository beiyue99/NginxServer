#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include<pthread.h>
#define PORT 8080
#define BUFFER_SIZE 1024

const char* message = "Hello My Client!";
char buffer[BUFFER_SIZE];
int server_fd, new_socket, valread;
//信息结构体
struct SockInfo
{
    struct sockaddr_in addr;
    int fd;
};
struct SockInfo infos[512];



void* working(void* arg);

int main()
{
    int opt = 1;
    int addrlen = sizeof(struct sockaddr_in);
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
    address.sin_port = htons(PORT);//将主机字节序（通常是小端字节序）转换为网络字节序（大端字节序）的函数
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
    }  //默认此函数也会阻塞
    //初始化结构体数组
    int max = sizeof(infos) / sizeof(infos[0]);
    for (int i = 0; i < max; ++i)
    {
        infos[i].fd = -1;
    }
    // 阻塞并等待客户端的连接
    while (1)
    {
        struct SockInfo* pinfo;
        for (int i = 0; i < max; ++i)
        {
            if (infos[i].fd == -1)
            {
                pinfo = &infos[i];
                break;
            }
        }
        if ((new_socket = accept(server_fd, (struct sockaddr*)&pinfo->addr, (socklen_t*)&addrlen)) == -1)
        {
            perror("accept failed");
            break; //跳出循环
        }
        pinfo->fd = new_socket;
        //创建子线程
        pthread_t tid;
        pthread_create(&tid, NULL, working, pinfo);
        pthread_detach(tid);
    }
    close(server_fd);
    return 0;
}




void* working(void* arg)
{
    struct SockInfo* pinfo = (struct SockInfo*)arg;
    if (pinfo == NULL) {
        fprintf(stderr, "Invalid pointer\n");
        return NULL;
    }
    //连接成功，打印客户端的IP和端口信息
    char ip[32];
    printf("Client IP: %s,Port: %d\n", inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, ip, sizeof(ip)), ntohs(pinfo->addr.sin_port));
    // 循环接收和发送消息
    while (1)
    {
        // 清空缓冲区
        memset(buffer, 0, BUFFER_SIZE);
        // 从客户端读取数据
        valread = read(pinfo->fd, buffer, BUFFER_SIZE);
        if (valread == -1) {
            perror("read failed");
            break;
        }
        else if (valread == 0) {
            // 客户端断开连接
            printf("Client disconnected\n");
            break;
        }
        // 接收缓冲区为空：当客户端发送了FIN（结束）包后，服务器端读取缓冲区中已经接收到的数据后，
        //再次调用read函数时，由于接收缓冲区已经为空，read函数将返回0。
        printf("Client: %s\n", buffer);

        // 判断通信是否结束
        if (strcmp(buffer, "quit") == 0) {
            break;  // 结束循环
        }

        // 向客户端发送消息
        if (send(pinfo->fd, message, strlen(message), 0) == -1) {
            perror("send failed");
            break; 
        }
    }

    // 关闭套接字
    close(pinfo->fd);
    pinfo->fd = -1;
    return NULL;
}