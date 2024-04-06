#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

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
    socklen_t client_addrlen = sizeof(client_addr);

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

    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(server_fd, &readset);
    int maxfd = server_fd;

    while (1)
    {
        fd_set tmp = readset;
        int ret = select(maxfd + 1, &tmp, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select failed");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i <= maxfd; ++i)
        {
            if (FD_ISSET(i, &tmp))
            {
                if (i == server_fd)
                {
                    // 接受客户端的连接
                    int cfd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addrlen);
                    if (cfd == -1)
                    {
                        perror("accept failed");
                        exit(EXIT_FAILURE);
                    }

                    FD_SET(cfd, &readset);
                    if (cfd > maxfd)
                        maxfd = cfd;

                    printf("New client connected. Socket fd: %d, IP: %s, Port: %d\n",cfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                }
                else
                {
                    // 从客户端读取数据
                    valread = read(i, buffer, BUFFER_SIZE);
                    if (valread == -1)
                    {
                        perror("read failed");
                        break;
                    }
                    else if (valread == 0)
                    {
                        // 客户端断开连接
                        printf("Client disconnected\n");
                        FD_CLR(i, &readset);
                        close(i);
                        break;
                    }

                    printf("Client: %s\n", buffer);

                    // 小写转大写
                    for (int j = 0; j < valread; ++j)
                    {
                        buffer[j] = toupper(buffer[j]);
                    }

                    printf("Modified message: %s\n", buffer);

                    // 判断通信是否结束
                    if (strcmp(buffer, "QUIT") == 0)
                    {
                        printf("Closing connection\n");
                        FD_CLR(i, &readset);
                        close(i);
                        break;
                    }

                    // 向客户端发送消息
                    if (send(i, buffer, strlen(buffer), 0) == -1)
                    {
                        perror("send failed");
                        break;
                    }

                    // 清空缓冲区
                    memset(buffer, 0, BUFFER_SIZE);
                }
            }
        }
    }

    // 关闭服务器套接字
    close(server_fd);

    return 0;
}

