#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

const char* message = "Hello My Client!";
char buffer[BUFFER_SIZE];
int server_fd, new_socket, valread;

int main()
{
    int opt = 1;
    int addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in address;
    struct sockaddr_in client_addr;
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
    printf("server is listening in 8080...\n");
    //监听，将客户端发来的请求放入队列，一个一个处理
    // 阻塞并等待客户端的连接
    while (1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen)) == -1)
        {
            perror("accept failed");
            continue; // 继续等待下一个连接
        }

        //连接成功，打印客户端的IP和端口信息
        char ip[32];
        printf("Client IP: %s, Port: %d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)), ntohs(client_addr.sin_port));

        // 循环接收和发送消息
        while (1)
        {
            // 从客户端读取数据
            valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread == -1) {
                perror("read failed");
                break; // 跳出内部循环，处理读取错误的情况
            }
            else if (valread == 0) {
                // 客户端断开连接
                printf("Client disconnected\n");
                break; // 跳出内部循环，处理客户端断开连接的情况
            }

            printf("Client: %s\n", buffer);

            // 判断通信是否结束
            if (strcmp(buffer, "quit") == 0) {
                break;  // 结束内部循环
            }

            // 向客户端发送消息
            if (send(new_socket, message, strlen(message), 0) == -1) {
                perror("send failed");
                break; // 跳出内部循环，处理发送失败的情况
            }
            // 清空缓冲区
            memset(buffer, 0, BUFFER_SIZE);
        }
        close(new_socket);
    }
    close(server_fd);
    return 0;
}