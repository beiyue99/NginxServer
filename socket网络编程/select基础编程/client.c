#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char message[BUFFER_SIZE] = { 0 };
    char buffer[BUFFER_SIZE] = { 0 };

    // 创建客户端套接字
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 将IPv4地址从点分十进制转换为二进制
    if (inet_pton(AF_INET, "192.168.43.27", &serv_addr.sin_addr.s_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    // 连接服务器
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    // 在循环中发送和接收消息
    while (1)
    {
        // 从用户输入读取消息
        printf("Enter message: ");
        fgets(message, BUFFER_SIZE, stdin);
        //使用 fgets 函数将输入的消息存储在 message 字符数组中，
        //最多读取 BUFFER_SIZE-1 个字符。然后，它将输入的消息打印回来
        //该函数从输入流（在这里是标准输入流 stdin）读取最多 n - 1 个字符，或者直到遇到换行符（'\n'）
        //该函数会在字符串的末尾自动添加一个终止符（'\0'）
        // fgets 会将换行符（'\n'）也读取并存储在字符串中，如果输入的行长度不超过 n-1，则换行符将是字符串的最后一个字符。
        //删除末尾的换行符
        message[strcspn(message, "\n")] = 0;

        // 向服务器发送消息
        if (send(sock, message, strlen(message), 0) == -1) {
            perror("send failed");
            exit(EXIT_FAILURE);
        }
        //printf("Message sent to the server successfully! \n");

         //检查是否收到退出指令
        if (strcmp(message, "quit") == 0) {
            printf("Quitting...\n");
            break;
        }

        // 从服务器接收响应
        valread = read(sock, buffer, BUFFER_SIZE);
        if (valread == -1) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }
        printf("Server: %s\n", buffer);
        // 清空缓冲区
        memset(buffer, 0, BUFFER_SIZE);
    }
    // 关闭套接字
    close(sock);

    return 0;
}
