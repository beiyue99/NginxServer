#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cctype>

#define MAXLINE 80
#define SERV_PORT 6666

int main()
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int sockfd;
    char buf[MAXLINE];
    std::string str(INET_ADDRSTRLEN, '\0');
    //INET_ADDRSTRLEN��һ���곣����ͨ������Ϊ16����ʾIPv4��ַ�ַ�������󳤶ȣ�
    //������β��null�ַ������������е��ַ�������ʼ��Ϊ'\0'��Ҳ����null�ַ���
    int n;
   // servaddr�ṹ���Ƿ������׽��ֵĵ�ַ��Ϣ������ṹ���������Ҫ�����ó�ָ����ֵ
   //�������κ�������������ݰ������Է��͵������ַ���������Ҫ�Ƚ�������ȫ������Ϊ�㣬Ȼ������������ֵ��
   // ����cliaddr�ṹ�壬������������ÿ�ν������ݰ�ʱ����ͻ��˵ĵ�ַ��Ϣ��
   // ����ṹ������ݶ��ᱻ������䣬���Բ���Ҫ���Ƚ�������Ϊ�㡣
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        return -1;
    }
    std::cout << "Accepting connections ..." << std::endl;

    while (true) 
    {
        cliaddr_len = sizeof(cliaddr);
        n = recvfrom(sockfd, buf, MAXLINE, 0, (struct sockaddr*)&cliaddr, &cliaddr_len);
        if (n < 0) 
        {
            std::cerr << "recvfrom error" << std::endl;
            break;
        }
        //�ر����ӵĸ��������,��Ϊudp��������Э��
        std::cout << "received from " << inet_ntop(AF_INET, &cliaddr.sin_addr, &str[0], str.size())
            << " at PORT " << ntohs(cliaddr.sin_port) << std::endl;

        for (int i = 0; i < n; i++)
            buf[i] = toupper(buf[i]);

        n = sendto(sockfd, buf, n, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
        if (n < 0) 
        {
            std::cerr << "sendto error" << std::endl;
            break;
        }
    }

    close(sockfd);
    return 0;
}
