
//evutil_socket_t 这个参数设为 - 1 通常表示你不想让这个事件与任何特定的文件描述符（在Unix - like系统）
//或套接字（在Windows）关联。在一些情况下，你可能只关心超时或者信号，而不是具体的读 / 写事件。
//
//举个例子，如果你想设置一个定时事件，你可能会这样做
//事件ev不与任何文件描述符或套接字关联，因此evutil_socket_t参数设为 - 1。
//这个事件的回调函数cb_func会在每过1秒时被调用。

#include <event2/event.h>
#include <sys/time.h>

void cb_func(evutil_socket_t fd, short what, void* arg) {
    printf("cb_func is call back!\n");
}




int main() {
    struct event_base* base = event_base_new();

    struct event* ev = event_new(base, -1, EV_PERSIST, cb_func, NULL);

    struct timeval one_sec = {1, 0};    
    //evutil_timerclear(&one_sec);  //清空为0,只调用一次就退出

    if (event_add(ev, &one_sec) != 0)
    {
        puts("Couldn't add event");
        return 1;
    }
    event_base_dispatch(base);

    event_free(ev);
    event_base_free(base);
    return 0;
}

