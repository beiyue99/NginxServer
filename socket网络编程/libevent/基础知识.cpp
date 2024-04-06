

struct event_base*  event_base_new(void);
声明创建了一个新的event_base。




struct event* event_new(struct event_base* base, evutil_socket_t fd, short what, event_callback_fn cb, void* arg);


evutil_socket_t fd : 需要监听的文件描述符。你可以为此参数传递 - 1，表示此事件不关联任何文件描述符
short what : 事件类型，可以是以下的一个或者多个，使用 | 运算符进行组合：
EV_READ（监听可读事件），
EV_WRITE（监听可写事件），
EV_PERSIST（持久事件，事件触发后不自动删除），
EV_ET（边缘触发模式），
EV_TIMEOUT（定时器事件），
EV_SIGNAL（信号事件）。
event_callback_fn cb : 事件回调函数，当事件触发时，此函数将被调用。
void* arg : 用户参数，传递给回调函数。
这个函数会返回一个新创建的事件，你可以使用 event_add() 将其添加到事件循环中:



int event_add(struct event* ev, const struct timeval* timeout);

const struct timeval* timeout : 这是一个可选的超时参数，如果提供了一个timeval结构体，
那么事件将在没有激活的情况下等待这么长的时间，然后就超时。如果设置为NULL，那么事件将等待直到被激活。







int event_base_dispatch(struct event_base* base);

它会阻塞并等待事件发生，一旦有事件发生
它就会调用相应的回调函数来处理这个事件。当所有的事件都被处理后，
会再次阻塞并等待新的事件。这个循环会一直持续，
直到没有更多的活动事件，或者直到调用 event_base_loopbreak() 或 event_base_loopexit() 函数。


如果函数成功地完成了所有的事件循环，那么返回 0。
如果没有注册事件，那么返回 1。
如果因为某种错误（比如提供了一个无效的 event_base而不能开始事件循环，那么返回 - 1。
如果事件循环被 event_base_loopbreak() 打断，那么返回 - 1。
如果事件循环被 event_base_loopexit() 设置的超时结束，那么返回 0。







int event_base_loop(struct event_base* base, int flags);
flags的取值：
#define EVLOOP_ONCE	0x01    只触发一次，如果事件没有被触发，阻塞等待
#define EVLOOP_NONBLOCK	0x02    非阻塞方式检测事件是否被触发，不管事件触发与否，都会立即返回



而大多数我们都调用libevent给我们提供的另外一个api：
int event_base_dispatch(struct event_base* base);
调用该函数，相当于没有设置标志位的event_base_loop。程序将会一直运行，直到没有需要检测的事件了，或者被结束循环的api终止。



int event_base_loopexit(struct event_base* base, const struct timeval* tv);
此函数用于安排事件循环在一段时间后退出。你可以通过设定struct timeval* tv指定的时间来安排事件循环何时退出。如果你设置的时间是0，
这意味着事件循环将会在处理完当前所有活动事件后，不再等待新事件的出现，立即退出。



int event_base_loopbreak(struct event_base* base);
此函数用于立即中断事件循环，无论是否还有活动事件未处理。一旦调用此函数，事件循环将在当前处理的事件完成后立即停止，
如果当前没有事件在处理，事件循环将立即结束。








读 / 写事件管理: 使用bufferevent可以自动处理读 / 写事件，使得开发者无需手动操作底层的socket read / write
输入 / 输出缓冲 : bufferevent内部提供了输入 / 输出缓冲区，使得数据的读取和写入更加方便。






struct bufferevent* bufferevent_socket_new(struct event_base* base, evutil_socket_t fd, enum bufferevent_options options);

base：事件的基础结构，这个基础结构是在你的程序开始时创建的，通常它是在主线程中创建的。

fd：一个套接字描述符，如果设置为 - 1，libevent 会在需要的时候创建一个新的套接字。

options：这是一个标志参数，可以接受以下几个选项：BEV_OPT_CLOSE_ON_FREE、BEV_OPT_DEFER_CALLBACKS和BEV_OPT_UNLOCK_CALLBACKS。
其中，BEV_OPT_CLOSE_ON_FREE 表示在 bufferevent 被释放时关闭其底层的套接字，
BEV_OPT_DEFER_CALLBACKS 表示延迟回调的执行，BEV_OPT_UNLOCK_CALLBACKS 表示在执行回调时不锁定 bufferevent。






void bufferevent_setcb(
    struct bufferevent* bufev,
    bufferevent_data_cb readcb,
    bufferevent_data_cb writecb,
    bufferevent_event_cb eventcb,
    void* cbarg);


readcb：这是读取操作的回调函数。当bufferevent的输入缓冲区中的数据满足读取条件（如达到低水位线）时，这个回调函数会被调用。

writecb：这是写入操作的回调函数。当bufferevent的输出缓冲区中的数据满足写入条件（如低于低水位线）时，这个回调函数会被调用。

eventcb：这是一些特殊事件的回调函数，如错误、连接断开、超时等。

cbarg：这是用户传入的参数，它会在调用上述的回调函数时作为参数传入。





void bufferevent_setwatermark(
    struct bufferevent* bufev,
    short events,
    size_t lowmark,
    size_t highmark);


bufev：你想要设置水位线的bufferevent。

events：表示要设置哪种事件的水位线，取值可以是EV_READ、EV_WRITE，也可以两者都设置（EV_READ | EV_WRITE）。

lowmark：表示低水位线的大小，当输入 / 输出缓冲区的数据量大于或等于这个值时，读 / 写回调函数会被触发。

highmark：表示高水位线的大小，对于读操作，当输入缓冲区的数据量超过这个值时，就不再读取更多的数据；对于写操作，
只有当输出缓冲区的数据量小于这个值时，才会触发写回调函数。如果你将highmark设置为0，那么就表示高水位线是无限的。





int bufferevent_socket_connect(struct bufferevent* bev,
    struct sockaddr* address,
    int addrlen);

bufferevent_socket_connect 是 libevent 库中的一个函数，用于建立 TCP 连接。这个函数是异步的，也就是说，
它并不会阻塞直到连接建立完成，而是在开始连接后就立即返回。你可以设置回调函数来处理连接完成或失败时的情况。
bufferevent_socket_connect函数会自动创建一个socket，并与指定的主机和端口进行连接




struct event_base* base = event_base_new();
struct bufferevent* bev;

// 创建一个新的bufferevent
bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

// 设置回调函数
bufferevent_setcb(bev, read_callback, write_callback, event_callback, NULL);

// 设置水位线
bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);

// 启动读/写事件
bufferevent_enable(bev, EV_READ | EV_WRITE);

// 连接服务器
struct sockaddr_in server_addr;
memset(&server_addr, 0, sizeof(server_addr));
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(port);
inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
bufferevent_socket_connect(bev, (struct sockaddr*)&server_addr, sizeof(server_addr));

// 启动事件循环
event_base_dispatch(base);




对于evconnlistener_new_bind函数，你只需要提供一个要监听的地址（struct sockaddr结构），
libevent会自动为你创建socket，并绑定到这个地址上。

struct evconnlistener*
    evconnlistener_new_bind(
        struct event_base* base,
        evconnlistener_cb cb,
        void* ptr,
        unsigned flags,
        int backlog,
        const struct sockaddr* sa,
        int socklen);

base：事件循环的基本对象。

cb：新连接到来时的回调函数。

ptr：传递给回调函数的参数。

flags：用于控制监听器行为的标志。常用的标志包括LEV_OPT_CLOSE_ON_FREE（在释放监听器时关闭底层的socket）
和LEV_OPT_REUSEABLE（允许在同一个地址上创建多个监听器）。

backlog：决定了内核为此socket排队的最大连接数。

sa：指定要监听的地址。

socklen：地址结构的大小。

evconnlistener_free(listener);//释放链接侦听器




struct event* evsignal_new(struct event_base* base, int sig, event_callback_fn cb, void* arg);


base：事件循环的基本对象。
sig：需要监听的信号。例如，你可以监听 SIGINT（通常由用户按下 Ctrl + C 触发）或 SIGTERM（通常用于请求程序终止）。
cb：当指定的信号发生时调用的回调函数。
arg：传递给回调函数的参数。


struct event* sigint_event;
sigint_event = evsignal_new(base, SIGINT, signal_cb, base);
在这个例子中，我们创建了一个新的信号事件，当用户按下 Ctrl + C（这会发出 SIGINT 信号）时，
调用 signal_cb 回调函数。我们也传递了 base 作为回调函数的参数，这通常在回调函数中用来控制事件循环。










struct evbuffer* input = bufferevent_get_input(bev);   //获取输入缓冲区
struct evbuffer* output = bufferevent_get_output(bev);//获取输出缓冲区

struct evbuffer* buf 是 libevent 库中的一个数据类型，表示一个事件缓冲区（event buffer）的对象。
    这个对象用于缓存网络数据，通常用于缓存从网络接收到的数据，或者等待发送到网络的数据。
    一个 evbuffer 对象可以动态增长，以适应添加到缓冲区的数据量。
    它还提供了很多操作函数，例如添加数据、删除数据、搜索数据、拷贝数据等。
/* 创建一个新的 evbuffer */
struct evbuffer* buf = evbuffer_new();

/* 添加数据到 evbuffer */
char data[] = "Hello, World!";
evbuffer_add(buf, data, strlen(data));

/* 从 evbuffer 中移除数据 */
char read_data[1024];
evbuffer_remove(buf, read_data, sizeof(read_data));

/* 释放 evbuffer */
evbuffer_free(buf);


int bufferevent_write(struct bufferevent* bufev, const void* data, size_t size);
bufferevent_write是将data的数据写到bufferevent的写缓冲区



size_t bufferevent_read(struct bufferevent* bufev, void* data, size_t size);
bufferevent_read 是将bufferevent的读缓冲区数据读到data中，同时将读到的数据从bufferevent的读缓冲清除。




int bufferevent_write_buffer(struct bufferevent* bufev, struct evbuffer* buf);

bufev：bufferevent 对象，你想写入数据的目标。

buf：evbuffer 对象，你要写入的数据来源。





int bufferevent_read_buffer(struct bufferevent* bufev, struct evbuffer* buf);
从 bufferevent 的输入缓冲区中取出数据，并添加到提供的 evbuffer 对象中。





int evbuffer_add_buffer(struct evbuffer* dst, struct evbuffer* src);
这个函数会把 src 缓冲区的所有内容移动到 dst 缓冲区，并清空 src 缓冲区





events 参数是一个标志集，用来表示bufferevent的状态。这个标志集可能包含以下几种状态：

BEV_EVENT_READING : 发生在读取操作中的错误。
BEV_EVENT_WRITING : 发生在写入操作中的错误。
BEV_EVENT_EOF : 对端socket已经关闭，即文件结束符EOF。
BEV_EVENT_ERROR : 在bufferevent中发生了一个非EOF的错误。
BEV_EVENT_TIMEOUT : 在读取或写入时超时。
BEV_EVENT_CONNECTED : 在bufferevent成功建立连接后触发。




evconnlistener_set_error_cb 是 libevent 库中的一个函数，它用于设置监听器（listener）的错误回调函数。
当网络监听器发生错误时，会调用这个回调函数。


void evconnlistener_set_error_cb(struct evconnlistener *lev, evconnlistener_errorcb cb);
参数说明：
struct evconnlistener *lev：你要设置错误回调的监听器。
evconnlistener_errorcb cb：错误回调函数，它的函数原型如下：


typedef void (*evconnlistener_errorcb)(struct evconnlistener *lev, void *ctx);
这个函数会在发生错误时被调用。其中，struct evconnlistener *lev 是出错的监听器，void *ctx 是你在创建监听器时传入的回调参数。
