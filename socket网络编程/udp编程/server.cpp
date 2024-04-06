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
    //INET_ADDRSTRLEN（一个宏常量，通常定义为16，表示IPv4地址字符串的最大长度，
    //包括结尾的null字符），并且所有的字符都被初始化为'\0'（也就是null字符）
    int n;
   // servaddr结构体是服务器套接字的地址信息。这个结构体的内容需要被设置成指定的值
   //（即，任何来自网络的数据包都可以发送到这个地址），因此需要先将其内容全部设置为零，然后再填充所需的值。
   // 对于cliaddr结构体，它的作用是在每次接收数据包时保存客户端的地址信息。
   // 这个结构体的内容都会被重新填充，所以不需要事先将其设置为零。
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
        //关闭连接的概念并不存在,因为udp是无连接协议
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
