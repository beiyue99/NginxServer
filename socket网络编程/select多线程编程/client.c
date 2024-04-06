#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
//"127.0.0.1" �Ǳ��ػػ���ַ��ͨ��������ͬһ̨������ϲ��Կͻ��˺ͷ��������롣
//������ƻ���ͬһ̨����������пͻ��˺ͷ��������뽫 "SERVER_IP" ����Ϊ "127.0.0.1"��
int main() {
    int clientSocket;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE];

    // �����ͻ����׽���
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // ���÷�������ַ�Ͷ˿�
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP);
    serverAddress.sin_port = htons(SERVER_PORT);

    // ���ӷ�����
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    while (1) {
        // �ӱ�׼�����ȡ�û�����
        printf("Enter message (or QUIT to exit): ");
        fgets(buffer, sizeof(buffer), stdin);

        // ������Ϣ��������
        if (send(clientSocket, buffer, strlen(buffer), 0) == -1) {
            perror("send failed");
            exit(EXIT_FAILURE);
        }

        // ���շ���������Ӧ
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

        // �ж��û��Ƿ�Ҫ�˳�
        if (strcmp(buffer, "QUIT\n") == 0) {
            printf("Closing connection\n");
            break;
        }
    }

    // �رտͻ����׽���
    close(clientSocket);
    return 0;
}
