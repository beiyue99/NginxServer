#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#define _EVENT_SIZE_ 1024
// server
int main(int argc, const char* argv[])
{
    // 创建监听的套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    
   
    // 绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9999);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  

    // 设置端口复用
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定端口
    int ret = bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));


    // 监听
    ret = listen(lfd, 64);


    // 创建一个epoll模型
    int epfd = epoll_create(100);


    struct epoll_event ev;
    ev.events = EPOLLIN;  
    ev.data.fd = lfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);


    struct epoll_event evs[_EVENT_SIZE_];
    int size = sizeof(evs) / sizeof(struct epoll_event);
    // 持续检测
    while (1)
    {
        int num = epoll_wait(epfd, evs, size, -1);  //返回就绪文件描述符个数，储存在evs数组里
        for (int i = 0; i < num; ++i)
        {
            // 取出当前的文件描述符
            int curfd = evs[i].data.fd;
            // 判断这个文件描述符是不是用于监听的
            if (curfd == lfd)
            {
                // 建立新的连接
                int cfd = accept(curfd, NULL, NULL);
                ev.events = EPOLLIN;     //加上EPOLLET  表示边缘触发  然后需要设置非阻塞属性
                ev.data.fd = cfd;
                ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
            }
            else
            {
                char buf[1024];
                memset(buf, 0, sizeof(buf));
                int len = recv(curfd, buf, sizeof(buf), 0);
                if (len == 0)
                {
                    printf("The client has disconnected!\n");
                    // 将这个文件描述符从epoll模型中删除
                    epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                    close(curfd);
                }
                else if (len > 0)
                {
                    // write(STDOUT_FILENO,buf,1en);   发送到当前终端，代替下面这行代码
                    printf("client say: %s\n", buf);
          /*        如果 buf 的最后一个字符不是空字符(\0)，则在使用 printf 函数输出时可能会导致乱码或意外的结果。
                    使用 write 函数直接输出 buf 的内容，不依赖字符串的结尾空字符。因此，即使最后一个字符不是空字符，
                    也不会出现乱码，write 函数会按照指定的长度输出。
                    为了避免出现乱码或意外结果，确保 buf 是以空字符结尾的字符串或者明确指定要输出的字节数。*/

                    send(curfd, buf, strlen(buf) + 1, 0);
                }
                else
                {
                    perror("recv");
                    exit(0);
                }
            }
        }
    }

    return 0;
}


