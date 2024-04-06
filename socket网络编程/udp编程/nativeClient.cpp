#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#define UNIX_PATH_MAX 108

int main(int argc, char* argv[])
{
    //创建unix流式套接字
    int cfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (cfd == -1) 
    {
        perror("socket error");
        return 1;
    }

    //如果不绑定,就是隐式绑定
    struct sockaddr_un myaddr;
    myaddr.sun_family = AF_UNIX;
    strcpy(myaddr.sun_path, "sock.c");
    int len1 = offsetof(struct sockaddr_un, sun_path) + strlen("sock.c");

    if (bind(cfd, (struct sockaddr*)&myaddr, len1) < 0)
    {
        perror("bind error");
        return 1;
    }

    //连接
    struct sockaddr_un seraddr;
    seraddr.sun_family = AF_UNIX;
    strcpy(seraddr.sun_path, "sock.s");

    if (connect(cfd, (struct sockaddr*)&seraddr, sizeof(seraddr)) < 0)
    {
        perror("connect error");
        return 1;
    }

    //读写
    while (1)
    {
        char buf[1500] = { 0 };
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n < 0)
        {
            perror("read error");
            break;
        }

        buf[n] = '\0';  // add string terminator

        if (send(cfd, buf, n, 0) < 0) 
        {
            perror("send error");
            break;
        }

        memset(buf, 0, sizeof(buf));

        n = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) 
        {
            if (n == 0) {
                printf("server closed connection\n");
            }
            else {
                perror("recv error");
            }
            break;
        }

        buf[n] = '\0';  // add string terminator

        printf("%s\n", buf);
    }


    close(cfd);
    unlink("sock.c");
    return 0;
}
