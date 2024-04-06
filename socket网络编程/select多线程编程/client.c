#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
//"127.0.0.1" 是本地回环地址，通常用于在同一台计算机上测试客户端和服务器代码。
//如果您计划在同一台计算机上运行客户端和服务器，请将 "SERVER_IP" 设置为 "127.0.0.1"。
int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE];

    // 创建客户端套接字
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址和端口
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddress.sin_port = htons(SERVER_PORT);

    // 连接服务器
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    while (1) {
        // 从标准输入读取用户输入
        printf("Enter message (or QUIT to exit): ");
        fgets(buffer, sizeof(buffer), stdin);

        // 发送消息到服务器
        if (send(clientSocket, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            exit(EXIT_FAILURE);
        }

        // 接收服务器的响应
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead == -1) {
            perror("read error");
            exit(EXIT_FAILURE);
        }
        else if (bytesRead == 0) {
            printf("Server disconnected\n");
            break;
        }

        buffer[bytesRead] = '\0';
        printf("Received response from server: %s\n", buffer);

        // 判断用户是否要退出
        if (strcmp(buffer, "QUIT\n") == 0) {
            printf("Closing connection\n");
            break;
        }
    }

    // 关闭客户端套接字
    close(clientSocket);
    return 0;
}
