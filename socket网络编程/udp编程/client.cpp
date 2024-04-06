//#include <iostream>
//#include <string>
//#include <cstring>
//#include <arpa/inet.h>
//#include <netinet/in.h>
//#include <unistd.h>
//#include <cctype>
//
//#define MAXLINE 80
//#define SERV_PORT 6666
//
//int main(int argc, char* argv[])
//{
//    struct sockaddr_in servaddr;
//    int sockfd, n;
//    char buf[MAXLINE];
//
//    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
//        std::cerr << "Socket creation failed" << std::endl;
//        return -1;
//    }
//
//    memset(&servaddr, 0, sizeof(servaddr));
//    servaddr.sin_family = AF_INET;
//    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
//    servaddr.sin_port = htons(SERV_PORT);
//
//    std::string line;
//    while (std::getline(std::cin, line)) {
//        line.copy(buf, MAXLINE);
//std::string::copy 函数来将 line 中的内容复制到 buf 中，但是 std::string::copy 函数并不在复制的内容后添加 \0，
//这可能会导致 sendto 函数发送了未定义的内容。我们应该直接使用 line.c_str() 来获取一个以 \0 结尾的字符指针，
//这样就能确保正确的行为。
//        n = sendto(sockfd, buf, line.length(), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
//        if (n < 0) {
//            std::cerr << "sendto error" << std::endl;
//            break;
//        }
//        n = recvfrom(sockfd, buf, MAXLINE, 0, NULL, 0);
//        if (n < 0) {
//            std::cerr << "recvfrom error" << std::endl;
//            break;
//        }
//        std::cout.write(buf, n);   //前n个字符输出到标准输出
//    }
//    close(sockfd);
//    return 0;
//}
//对于 std::cout.write(buf, n) ，由于 recvfrom 函数并不会在接收到的数据后添加 \0 ，
//可能会导致在屏幕上显示未定义的内容。正确的做法是先将接收到的数据转换为 std::string ，然后再输出。

//std::string的c_str()成员函数返回的C风格字符串一定会以'\0'字符结尾。
//这是因为C语言的字符串标准要求字符串以'\0'字符结尾，以此来标示字符串的结束。



#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cctype>

#define MAXLINE 80
#define SERV_PORT 6666

int main(int argc, char* argv[])
{
    struct sockaddr_in servaddr;
    int sockfd, n;
    char buf[MAXLINE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
    {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    servaddr.sin_port = htons(SERV_PORT);

    std::string line;
    while (std::getline(std::cin, line))
    {
        n = sendto(sockfd, line.c_str(), line.length(), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        if (n < 0) 
        {
            std::cerr << "sendto error" << std::endl;
            break;
        }
        n = recvfrom(sockfd, buf, MAXLINE, 0, NULL, 0);
        if (n < 0) 
        {
            std::cerr << "recvfrom error" << std::endl;
            break;
        }
        std::string recvStr(buf, n);
        std::cout << recvStr << std::endl;
    }
    close(sockfd);
    return 0;
}
