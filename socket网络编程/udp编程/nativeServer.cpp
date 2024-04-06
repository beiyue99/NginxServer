#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>

#define UNIX_PATH_MAX 108

int main(int argc, char* argv[])
{
    //����unix��ʽ�׽���
    int lfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (lfd == -1) {
        std::cerr << "socket error: " << strerror(errno) << std::endl;
        return 1;
    }

    //��
    struct sockaddr_un myaddr;
    myaddr.sun_family = AF_UNIX;
    strcpy(myaddr.sun_path, "sock.s");
    int len1 = offsetof(struct sockaddr_un, sun_path) + strlen(myaddr.sun_path);
    if (bind(lfd, (struct sockaddr*)&myaddr, len1) == -1)
    {
        std::cerr << "bind error: " << strerror(errno) << std::endl;
        return 1;
    }

    //����
    if (listen(lfd, 128) == -1)
    {
        std::cerr << "listen error: " << strerror(errno) << std::endl;
        return 1;
    }

    //��ȡ
    struct sockaddr_un cliaddr;
    socklen_t len2 = sizeof(cliaddr);
    int cfd = accept(lfd, (struct sockaddr*)&cliaddr, &len2);
    if (cfd == -1) 
    {
        std::cerr << "accept error: " << strerror(errno) << std::endl;
        return 1;
    }

    std::cout << "new client file " << cliaddr.sun_path << std::endl;

    //��д
    char buf[1500];
    while (1)
    {
        int n = recv(cfd, buf, sizeof(buf) - 1, 0);  // leave room for '\0'
        if (n <= 0)
        {
            if (n == 0) 
            {
                std::cout << "client closed connection" << std::endl;
            }
            else 
            {
                std::cerr << "recv error: " << strerror(errno) << std::endl;
            }

            break;
        }
//��ʹ�� recv() ���������ַ�������ʱ����Ҫע������ַ�����ֹ�� '\0'��
//recv() ���������Զ������ֹ����������ֱ�Ӵ�ӡ���յ������ݣ����ܻ��������⡣
//һ�ֽ���������ڵ��� recv() ����ʱ�������ջ������Ĵ�С��һ��Ȼ���ڽ������ݺ��ֶ������ֹ����
        buf[n] = '\0';  // add string terminator
        std::cout << buf << std::endl;
        if (send(cfd, buf, n, 0) == -1)
        {
            std::cerr << "send error: " << strerror(errno) << std::endl;
            break;
        }
    }

    //�ر����Ӻ��ļ�������
    close(cfd);
    close(lfd);

    //ɾ���׽����ļ�
    unlink("sock.s");

    return 0;
}


