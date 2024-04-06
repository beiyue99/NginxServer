#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <ctype.h>

static const char MESSAGE[] = "Hello, World!\n";

static const int PORT = 9995;

static void listener_cb(struct evconnlistener*, evutil_socket_t,struct sockaddr*, int socklen, void*);
static void conn_writecb(struct bufferevent*, void*);
static void conn_readcb(struct bufferevent*, void*);
static void conn_eventcb(struct bufferevent*, short, void*);
static void signal_cb(evutil_socket_t, short, void*);

int main(int argc, char** argv)
{
	struct event_base* base;//根节点定义
	struct evconnlistener* listener;//监听器定义
	struct event* signal_event;//信号事件

	struct sockaddr_in sin;

	base = event_base_new();//创建根节点
	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);

	//创建监听器-端口复用-关闭自动释放
	listener = evconnlistener_new_bind(base, listener_cb, (void*)base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,    //-1是系统默认的连接队列大小
		(struct sockaddr*)&sin,
		sizeof(sin));

	if (!listener) {
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}

	//定义信号回调事件 -SIGINT
	signal_event = evsignal_new(base, SIGINT, signal_cb, (void*)base);
	//event_add上树 -开始监听信号事件
	if (!signal_event || event_add(signal_event, NULL) < 0) {
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}

	//循环等待事件
	event_base_dispatch(base);
	//释放链接侦听器
	evconnlistener_free(listener);
	event_free(signal_event);
	event_base_free(base);

	printf("done\n");
	return 0;
}
//链接监听器帮助处理了 accept连接，得到新的文件描述符，作为参数传入
static void
listener_cb(struct evconnlistener* listener, evutil_socket_t fd,
	struct sockaddr* sa, int socklen, void* user_data)
{
	printf("---call------%s----\n", __FUNCTION__);
	struct event_base* base = user_data;
	struct bufferevent* bev;//定义bufferevent事件

	//创建新的bufferevent事件，对应的与客户端通信的socket
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		fprintf(stderr, "Error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}
	//设置回调函数 只设置了写回调和事件产生回调
	bufferevent_setcb(bev, conn_readcb, conn_writecb, conn_eventcb, NULL);
	//启用读写缓冲区
	bufferevent_enable(bev, EV_WRITE | EV_READ);
	//禁用读缓冲
	//bufferevent_disable(bev, EV_READ);
	//将MESSAGE 写到输出缓冲区
	//bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
}

//自定义读回调函数
static void
conn_readcb(struct bufferevent* bev, void* user_data)
{
	printf("---calll-----%s\n", __FUNCTION__);
	//何时被触发？读入缓冲区有数据的时候，非底层的
	char buf[256] = { 0 };
	size_t ret = bufferevent_read(bev, buf, sizeof(buf));
	if (ret > 0) {
		//转为大写
		int i;
		for (i = 0; i < ret; i++) {
			buf[i] = toupper(buf[i]);
		}
		//写到bufferevent的输出缓冲区
		bufferevent_write(bev, buf, ret);
	}
}

static void
conn_writecb(struct bufferevent* bev, void* user_data)
{

	printf("---call------%s----\n", __FUNCTION__);
	struct evbuffer* output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0) {
		printf("flushed answer\n");   //表示已经将所有待发送的数据从输出缓冲区刷新出去，发送到目标的接收端
		//	bufferevent_free(bev);
	}
}

static void
conn_eventcb(struct bufferevent* bev, short events, void* user_data)
{
	printf("---call------%s----\n", __FUNCTION__);
	if (events & BEV_EVENT_EOF) {
		printf("Connection closed.\n");
	}
	else if (events & BEV_EVENT_ERROR) {
		printf("Got an error on the connection: %s\n",
			strerror(errno));/*XXX win32*/
	}
	/* None of the other events can happen here, since we haven't enabled
	 * timeouts */
	bufferevent_free(bev);
}

static void
signal_cb(evutil_socket_t sig, short events, void* user_data)
{
	printf("---call------%s----\n", __FUNCTION__);
	struct event_base* base = user_data;
	struct timeval delay = { 2, 0 };//设置延迟时间 2s

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);//延时2s退出
}