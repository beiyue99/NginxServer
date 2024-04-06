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
//std::string::copy �������� line �е����ݸ��Ƶ� buf �У����� std::string::copy ���������ڸ��Ƶ����ݺ���� \0��
//����ܻᵼ�� sendto ����������δ��������ݡ�����Ӧ��ֱ��ʹ�� line.c_str() ����ȡһ���� \0 ��β���ַ�ָ�룬
//��������ȷ����ȷ����Ϊ��
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
//        std::cout.write(buf, n);   //ǰn���ַ��������׼���
//    }
//    close(sockfd);
//    return 0;
//}
//���� std::cout.write(buf, n) ������ recvfrom �����������ڽ��յ������ݺ���� \0 ��
//���ܻᵼ������Ļ����ʾδ��������ݡ���ȷ���������Ƚ����յ�������ת��Ϊ std::string ��Ȼ���������

//std::string��c_str()��Ա�������ص�C����ַ���һ������'\0'�ַ���β��
//������ΪC���Ե��ַ�����׼Ҫ���ַ�����'\0'�ַ���β���Դ�����ʾ�ַ����Ľ�����



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
