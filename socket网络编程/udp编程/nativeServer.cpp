#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>

#define UNIX_PATH_MAX 108

int main(int argc, char* argv[])
{
    //创建unix流式套接字
    int lfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (lfd == -1) {
        std::cerr << "socket error: " << strerror(errno) << std::endl;
        return 1;
    }

    //绑定
    struct sockaddr_un myaddr;
    myaddr.sun_family = AF_UNIX;
    strcpy(myaddr.sun_path, "sock.s");
    int len1 = offsetof(struct sockaddr_un, sun_path) + strlen(myaddr.sun_path);
    if (bind(lfd, (struct sockaddr*)&myaddr, len1) == -1)
    {
        std::cerr << "bind error: " << strerror(errno) << std::endl;
        return 1;
    }

    //监听
    if (listen(lfd, 128) == -1)
    {
        std::cerr << "listen error: " << strerror(errno) << std::endl;
        return 1;
    }

    //提取
    struct sockaddr_un cliaddr;
    socklen_t len2 = sizeof(cliaddr);
    int cfd = accept(lfd, (struct sockaddr*)&cliaddr, &len2);
    if (cfd == -1) 
    {
        std::cerr << "accept error: " << strerror(errno) << std::endl;
        return 1;
    }

    std::cout << "new client file " << cliaddr.sun_path << std::endl;

    //读写
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
//在使用 recv() 函数接收字符串数据时，需要注意添加字符串终止符 '\0'。
//recv() 函数不会自动添加终止符，因此如果直接打印接收到的数据，可能会遇到问题。
//一种解决方法是在调用 recv() 函数时，将接收缓冲区的大小减一，然后在接收数据后手动添加终止符。
        buf[n] = '\0';  // add string terminator
        std::cout << buf << std::endl;
        if (send(cfd, buf, n, 0) == -1)
        {
            std::cerr << "send error: " << strerror(errno) << std::endl;
            break;
        }
    }

    //关闭连接和文件描述符
    close(cfd);
    close(lfd);

    //删除套接字文件
    unlink("sock.s");

    return 0;
}


