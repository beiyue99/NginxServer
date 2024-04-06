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

    // �����������׽���
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // ���÷�������ַ�Ͷ˿�
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // �󶨷�������ַ�Ͷ˿�
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // ������������
    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listeningon port %d\n", PORT);


    // �ͻ�������
    ClientInfo* clientInfoArray[MAX_CLIENTS];
    int clientCount = 0;

    // ��ʼ��������
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
        // ����������׽����пɶ��¼�����ʾ���µ���������accept��������
        if (FD_ISSET(serverSocket, &tmp))
        {
            clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        }
        // �����µĿͻ�����Ϣ
        ClientInfo* newClient = (ClientInfo*)malloc(sizeof(ClientInfo));
        newClient->clientSocket = clientSocket;
        newClient->clientAddress = clientAddress;


        // ��ӵ��ͻ�������
        pthread_mutex_lock(&mutex);  // ����
        if (clientCount < MAX_CLIENTS)
        {
            clientInfoArray[clientCount] = newClient;
            clientCount++;
            printf("New client connected: IP %s, Port %d\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
        }
        // ����ͻ��������������ܾ����Ӳ��ر��׽���
        else
        {
            printf("Maximum number of clients reached. Connection rejected.\n");
            close(clientSocket);
            free(newClient);
        }


        // ��ӿͻ����׽��ֵ� readSet��ͬʱ��������ļ�������
        for (int i = 0; i < clientCount; i++)
        {
            int clientSock = clientInfoArray[i]->clientSocket;
            FD_SET(clientSock, &readSet);
            if (clientSock > maxFd)
            {
                maxFd = clientSock;
            }
        }


        //����ͨѶ���ļ��������������ĸ����ھ���״̬��Ȼ�󴴽��̴߳������ǣ�clientCount--
        for (int i = 0; i < clientCount; i++)
        {
            int clientSock = clientInfoArray[i]->clientSocket;
            if (FD_ISSET(clientSock, &readSet))
            {
                // ����ͻ�������
                pthread_t tid;
                ClientInfo* clientInfo = clientInfoArray[i];
                pthread_create(&tid, NULL, handleClient, clientInfo);
                pthread_detach(tid);

                // �ӿͻ����������Ƴ��ͻ���
                memmove(clientInfoArray + i, clientInfoArray + i + 1, (clientCount - i - 1) * sizeof(ClientInfo*));
                clientCount--;
                i--;
                // ���ļ�������������ɾ���ѶϿ����ӵĿͻ����׽���
                FD_CLR(clientSock, &readSet);
            }
        }
        pthread_mutex_unlock(&mutex);  // ����
    }














        // �رշ������׽���
        close(serverSocket);
        // ���ٻ�����
        pthread_mutex_destroy(&mutex);

        return 0;
    }



void handleClient(void*arg)
{
    ClientInfo* clientInfo = (ClientInfo*)arg;
    int clientSocket = clientInfo->clientSocket;
    struct sockaddr_in clientAddress = clientInfo->clientAddress;
    char buffer[BUFFER_SIZE];

    // ��ӡ�ͻ���������Ϣ
    printf("Client connected: IP %s, Port %d\n",
        inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

    while (1) {
        // ��ȡ�ͻ��˷��͵�����
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

        // �޸�����
        for (int i = 0; i < bytesRead; i++) {
            buffer[i] = toupper(buffer[i]);
        }

        printf("Modified message: %s\n", buffer);

        // �ж�ͨ���Ƿ����
        if (strcmp(buffer, "QUIT") == 0) {
            printf("Closing connection\n");
            break;
        }

        // ��ͻ��˷�����Ϣ
        if (send(clientSocket, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            break;
        }
    }

    // �رտͻ����׽���
    close(clientSocket);
    free(clientInfo);
    return NULL;
}