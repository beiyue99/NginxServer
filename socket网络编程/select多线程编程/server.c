#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include<ctype.h>
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define PORT 8080

typedef struct {
    int clientSocket;
    struct sockaddr_in clientAddress;
} ClientInfo;

pthread_mutex_t mutex;

void* handleClient(void* arg);


int main() 
{
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);

    // 创建服务器套接字
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址和端口
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // 绑定服务器地址和端口
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听连接请求
    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listeningon port %d\n", PORT);


    // 客户端数组
    ClientInfo* clientInfoArray[MAX_CLIENTS];
    int clientCount = 0;

    // 初始化互斥锁
    if (pthread_mutex_init(&mutex, NULL) != 0) {
        perror("mutex init failed");
        exit(EXIT_FAILURE);
    }








    fd_set readSet;
    int maxFd;
    FD_ZERO(&readSet);
    FD_SET(serverSocket, &readSet);
    maxFd = serverSocket;
    while (1)
    {
        fd_set tmp = readSet;
        select(maxFd + 1, &tmp, NULL, NULL, NULL);
        // 如果服务器套接字有可读事件，表示有新的连接请求，accept建立连接
        if (FD_ISSET(serverSocket, &tmp))
        {
            clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        }
        // 创建新的客户端信息
        ClientInfo* newClient = (ClientInfo*)malloc(sizeof(ClientInfo));
        newClient->clientSocket = clientSocket;
        newClient->clientAddress = clientAddress;


        // 添加到客户端数组
        pthread_mutex_lock(&mutex);  // 加锁
        if (clientCount < MAX_CLIENTS)
        {
            clientInfoArray[clientCount] = newClient;
            clientCount++;
            printf("New client connected: IP %s, Port %d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        }
        // 如果客户端数组已满，拒绝连接并关闭套接字
        else
        {
            printf("Maximum number of clients reached. Connection rejected.\n");
            close(clientSocket);
            free(newClient);
        }


        // 添加客户端套接字到 readSet，同时更新最大文件描述符
        for (int i = 0; i < clientCount; i++)
        {
            int clientSock = clientInfoArray[i]->clientSocket;
            FD_SET(clientSock, &readSet);
            if (clientSock > maxFd)
            {
                maxFd = clientSock;
            }
        }


        //遍历通讯的文件描述符，看看哪个处于就绪状态。然后创建线程处理他们，clientCount--
        for (int i = 0; i < clientCount; i++)
        {
            int clientSock = clientInfoArray[i]->clientSocket;
            if (FD_ISSET(clientSock, &readSet))
            {
                // 处理客户端请求
                pthread_t tid;
                ClientInfo* clientInfo = clientInfoArray[i];
                pthread_create(&tid, NULL, handleClient, clientInfo);
                pthread_detach(tid);

                // 从客户端数组中移除客户端
                memmove(clientInfoArray + i, clientInfoArray + i + 1, (clientCount - i - 1) * sizeof(ClientInfo*));
                clientCount--;
                i--;
                // 从文件描述符集合中删除已断开连接的客户端套接字
                FD_CLR(clientSock, &readSet);
            }
        }
        pthread_mutex_unlock(&mutex);  // 解锁
    }














        // 关闭服务器套接字
        close(serverSocket);
        // 销毁互斥锁
        pthread_mutex_destroy(&mutex);

        return 0;
    }



void handleClient(void*arg)
{
    ClientInfo* clientInfo = (ClientInfo*)arg;
    int clientSocket = clientInfo->clientSocket;
    struct sockaddr_in clientAddress = clientInfo->clientAddress;
    char buffer[BUFFER_SIZE];

    // 打印客户端连接信息
    printf("Client connected: IP %s, Port %d\n",
        inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

    while (1) {
        // 读取客户端发送的数据
        int bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        if (bytesRead == -1) {
            perror("read error");
            break;
        }
        else if (bytesRead == 0) {
            printf("Client disconnected\n");
            break;
        }

        buffer[bytesRead] = '\0';
        printf("Received message from client: %s\n", buffer);

        // 修改数据
        for (int i = 0; i < bytesRead; i++) {
            buffer[i] = toupper(buffer[i]);
        }

        printf("Modified message: %s\n", buffer);

        // 判断通信是否结束
        if (strcmp(buffer, "QUIT") == 0) {
            printf("Closing connection\n");
            break;
        }

        // 向客户端发送消息
        if (send(clientSocket, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            break;
        }
    }

    // 关闭客户端套接字
    close(clientSocket);
    free(clientInfo);
    return NULL;
}