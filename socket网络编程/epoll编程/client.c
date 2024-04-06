#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#define PORT 9999
#define BUFFER_SIZE 1024
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

int main()
{
    // 1. ��������ͨ�ŵ��׽���
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        exit(0);
    }

    // 2. ���ӷ�����
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;     // ipv4
    addr.sin_port = htons(PORT);   // �����������Ķ˿�, �ֽ���Ӧ���������ֽ���
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
    int ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect");
        exit(0);
    }

    // ͨ��
    int i = 0;
    while (1)
    {
        char recvBuf[1024] = { 0 };
        // д����
        sprintf(recvBuf, "data: %d\n", i++);
        write(fd, recvBuf, strlen(recvBuf) + 1);
        int len = read(fd, recvBuf, sizeof(recvBuf));
        //�ͻ���ִ�� write ����֮������ִ�� read �����������ܱ�֤������ȡ����������������Ӧ���ݡ�
        //�����������û�����ü��������ݲ�������Ӧ����ô read �������ܻ�ȴ�һ��ʱ���һֱ������ֱ�����ݵ��������ʱ
        if (len == -1)
        {
            perror("read error");
            exit(1);
        }
        printf("recv buf: %s\n", recvBuf);
        sleep(1);
    }

    // �ͷ���Դ
    close(fd);
    return 0;
}
