#include <event2/event.h>
#include <unistd.h>

void cb_func(evutil_socket_t fd, short what, void* arg)
{
    char buf[1024];
   /* while (fgets(buf, sizeof(buf), stdin) != NULL) {
        // 处理读取到的数据
        printf("read from stdin : %s\n", buf);
    }*/
    fgets( buf, sizeof(buf), stdin);
    printf("read from stdin : %s\n", buf);
    const char* data = arg;
    printf("Got an event on socket %d:%s%s%s%s [%s]\n",
        (int)fd,
        (what & EV_TIMEOUT) ? " timeout" : "", //如果 what 中的 EV_TIMEOUT 位被设置，那么 what & EV_TIMEOUT 的结果就不为0
        (what & EV_READ) ? " read" : "",
        (what & EV_WRITE) ? " write" : "",
        (what & EV_SIGNAL) ? " signal" : "",
        data);
}

int main()
{
    struct event_base* base = event_base_new();
    evutil_socket_t fd = STDIN_FILENO;

/* 我们正在监听的事件类型是：可读事件，并且是持久事件 */
    short what = EV_READ | EV_PERSIST;

    /* 设置一个回调函数 */
    event_callback_fn cb = cb_func;

    /* 创建一个新的事件 */
    struct event* ev = event_new(base, fd, what, cb, "User Argument");

    /* 添加事件到事件循环 */
    event_add(ev, NULL);

    /* 开始事件循环 */
    event_base_dispatch(base);

    /* 清理 */
    event_free(ev);
    event_base_free(base);

    return 0;
}
