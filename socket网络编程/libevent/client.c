#include <stdio.h>
#include <unistd.h>
#include <event.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXLINE 1024

// 定义一个新的结构体类型
struct sock_ev 
{
    struct event_base* base;
    int sockfd;
};

void recv_cb(int sockfd, short event, void* arg)
{
    char buf[MAXLINE] = { 0 };
    int len = recv(sockfd, buf, sizeof(buf), 0);
    if (len <= 0)
    {
        if (len < 0)
            perror("Read error");
        else
            printf("Server closed connection\n");

        close(sockfd);
        event_base_loopexit((struct event_base*)arg, NULL);
        return;
    }

    printf("Received from server: %s\n", buf);
}

void input_cb(int fd, short event, void* arg)
{
    struct sock_ev* sock_ev = (struct sock_ev*)arg;
    char buf[MAXLINE] = { 0 };
    if (fgets(buf, sizeof(buf), stdin) == NULL)
    //fgets是用于从标准输入（stdin）读取数据的。在这种情况下，
    //当没有更多的输入时（例如，如果用户按下了Ctrl+D来表示输入的结束，在Windows系统中，是Ctrl+Z），fgets会返回NULL
    {
        printf("No input, exiting\n");
        event_base_loopbreak(sock_ev->base);
        return;
    }

    int len = strlen(buf);
    if (send(sock_ev->sockfd, buf, len, 0) < 0)
    {
        perror("Send error");
        close(sock_ev->sockfd);
        event_base_loopbreak(sock_ev->base);
    }
}

int main(int argc, char* argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Socket error");
        return 1;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(8888);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Connect error");
        close(sockfd);
        return 1;
    }

    struct event_base* base = event_base_new();
    if (!base)
    {
        perror("Could not create event base");
        close(sockfd);
        return 1;
    }

    struct sock_ev sock_ev;
    sock_ev.base = base;
    sock_ev.sockfd = sockfd;

    struct event* ev_recv = event_new(base, sockfd, EV_READ | EV_PERSIST, recv_cb, base);
    struct event* ev_input = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST, input_cb, &sock_ev);

    event_add(ev_recv, NULL);
    event_add(ev_input, NULL);

    event_base_dispatch(base);

    event_free(ev_recv);
    event_free(ev_input);
    event_base_free(base);
    close(sockfd);

    return 0;
}
