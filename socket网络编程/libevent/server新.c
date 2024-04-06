#include <event2/event.h>
#include <event2/bufferevent.h>
#include <sys/socket.h>
#include <string.h>
#include<event2/buffer.h>
void echo_read_cb(struct bufferevent* bev, void* ctx) {
    /* This callback is invoked when there is data to read on bev. */
    struct evbuffer* input = bufferevent_get_input(bev);   //获取输入缓冲区
    struct evbuffer* output = bufferevent_get_output(bev);//获取输出缓冲区

    /* Copy all the data from the input buffer to the output buffer. */
    evbuffer_add_buffer(output, input);
}

void echo_event_cb(struct bufferevent* bev, short events, void* ctx) {
    if (events & BEV_EVENT_ERROR)
        perror("Error from bufferevent");
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
        bufferevent_free(bev);
    }
    //BEV_EVENT_EOF：当 bufferevent 的套接字已经到达文件结束符（EOF）时，这个事件被触发。这通常表示 TCP 连接已经被对方关闭。
    //BEV_EVENT_ERROR：当 bufferevent 发生错误时，这个事件被触发。这可能包括套接字错误、读写操作失败等各种类型的错误。
}

void accept_conn_cb(evutil_socket_t listener, short event, void* arg) {
    struct event_base* base = arg;
    struct sockaddr_storage ss;  //储存任意socket类型的结构体
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        perror("accept");
    }
    else
    {
        printf("new client connect :%d\n", fd);
        struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
        bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);
        bufferevent_enable(bev, EV_READ | EV_WRITE);
    }
}

int main(int argc, char** argv) {
    struct event_base* base;
    base = event_base_new();
    if (!base) {
        puts("Couldn't open event base");
        return 1;
    }
    evutil_socket_t listener;
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0);
    sin.sin_port = htons(8888);
    listener = socket(AF_INET, SOCK_STREAM, 0);
    bind(listener, (struct sockaddr*)&sin, sizeof(sin));
    listen(listener, 16);




    struct event* listener_event;
    listener_event = event_new(base, listener, EV_READ | EV_PERSIST, accept_conn_cb, base);
    event_add(listener_event, NULL);

    event_base_dispatch(base);

    return 0;
}

