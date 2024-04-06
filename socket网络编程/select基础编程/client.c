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

    // �����ͻ����׽���
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // ���÷�������ַ�Ͷ˿�
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // ��IPv4��ַ�ӵ��ʮ����ת��Ϊ������
    if (inet_pton(AF_INET, "192.168.43.27", &serv_addr.sin_addr.s_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    // ���ӷ�����
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    // ��ѭ���з��ͺͽ�����Ϣ
    while (1)
    {
        // ���û������ȡ��Ϣ
        printf("Enter message: ");
        fgets(message, BUFFER_SIZE, stdin);
        //ʹ�� fgets �������������Ϣ�洢�� message �ַ������У�
        //����ȡ BUFFER_SIZE-1 ���ַ���Ȼ�������������Ϣ��ӡ����
        //�ú��������������������Ǳ�׼������ stdin����ȡ��� n - 1 ���ַ�������ֱ���������з���'\n'��
        //�ú��������ַ�����ĩβ�Զ����һ����ֹ����'\0'��
        // fgets �Ὣ���з���'\n'��Ҳ��ȡ���洢���ַ����У����������г��Ȳ����� n-1�����з������ַ��������һ���ַ���
        //ɾ��ĩβ�Ļ��з�
        message[strcspn(message, "\n")] = 0;

        // �������������Ϣ
        if (send(sock, message, strlen(message), 0) == -1) {
            perror("send failed");
            exit(EXIT_FAILURE);
        }
        //printf("Message sent to the server successfully! \n");

         //����Ƿ��յ��˳�ָ��
        if (strcmp(message, "quit") == 0) {
            printf("Quitting...\n");
            break;
        }

        // �ӷ�����������Ӧ
        valread = read(sock, buffer, BUFFER_SIZE);
        if (valread == -1) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }
        printf("Server: %s\n", buffer);
        // ��ջ�����
        memset(buffer, 0, BUFFER_SIZE);
    }
    // �ر��׽���
    close(sock);

    return 0;
}
