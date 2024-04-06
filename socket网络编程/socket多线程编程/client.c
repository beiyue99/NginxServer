#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#define PORT 8080
#define BUFFER_SIZE 1024
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
    // 1. 创建用于通信的套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(0);
    }

    // 2. 连接服务器
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;     // ipv4
    addr.sin_port = htons(8080);   // 服务器监听的端口, 字节序应该是网络字节序
    inet_pton(AF_INET, "192.168.43.27", &addr.sin_addr.s_addr);
    int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect");
        exit(0);
    }

    // 通信
    int i = 0;
    while (1)
    {
        // 读数据
        char recvBuf[1024] = { 0 };
        // 写数据
        sprintf(recvBuf, "data: %d\n", i++);
        write(fd, recvBuf, strlen(recvBuf) + 1);
        int len = read(fd, recvBuf, sizeof(recvBuf));
        //客户端执行 write 函数之后立即执行 read 函数，并不能保证立即读取到服务器处理后的响应数据。
        //如果服务器还没有来得及处理数据并发送响应，那么 read 函数可能会等待一段时间或一直阻塞，直到数据到达或发生超时
        if (len == -1)
        {
            perror("read error");
            exit(1);
        }
        printf("recv buf: %s\n", recvBuf);
        sleep(1);
    }

    // 释放资源
    close(fd);

    return 0;
}
