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
	struct event_base* base;//���ڵ㶨��
	struct evconnlistener* listener;//����������
	struct event* signal_event;//�ź��¼�

	struct sockaddr_in sin;

	base = event_base_new();//�������ڵ�
	if (!base) {
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORT);

	//����������-�˿ڸ���-�ر��Զ��ͷ�
	listener = evconnlistener_new_bind(base, listener_cb, (void*)base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,    //-1��ϵͳĬ�ϵ����Ӷ��д�С
		(struct sockaddr*)&sin,
		sizeof(sin));

	if (!listener) {
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}

	//�����źŻص��¼� -SIGINT
	signal_event = evsignal_new(base, SIGINT, signal_cb, (void*)base);
	//event_add���� -��ʼ�����ź��¼�
	if (!signal_event || event_add(signal_event, NULL) < 0) {
		fprintf(stderr, "Could not create/add a signal event!\n");
		return 1;
	}

	//ѭ���ȴ��¼�
	event_base_dispatch(base);
	//�ͷ�����������
	evconnlistener_free(listener);
	event_free(signal_event);
	event_base_free(base);

	printf("done\n");
	return 0;
}
//���Ӽ��������������� accept���ӣ��õ��µ��ļ�����������Ϊ��������
static void
listener_cb(struct evconnlistener* listener, evutil_socket_t fd,
	struct sockaddr* sa, int socklen, void* user_data)
{
	printf("---call------%s----\n", __FUNCTION__);
	struct event_base* base = user_data;
	struct bufferevent* bev;//����bufferevent�¼�

	//�����µ�bufferevent�¼�����Ӧ����ͻ���ͨ�ŵ�socket
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		fprintf(stderr, "Error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}
	//���ûص����� ֻ������д�ص����¼������ص�
	bufferevent_setcb(bev, conn_readcb, conn_writecb, conn_eventcb, NULL);
	//���ö�д������
	bufferevent_enable(bev, EV_WRITE | EV_READ);
	//���ö�����
	//bufferevent_disable(bev, EV_READ);
	//��MESSAGE д�����������
	//bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
}

//�Զ�����ص�����
static void
conn_readcb(struct bufferevent* bev, void* user_data)
{
	printf("---calll-----%s\n", __FUNCTION__);
	//��ʱ�����������뻺���������ݵ�ʱ�򣬷ǵײ��
	char buf[256] = { 0 };
	size_t ret = bufferevent_read(bev, buf, sizeof(buf));
	if (ret > 0) {
		//תΪ��д
		int i;
		for (i = 0; i < ret; i++) {
			buf[i] = toupper(buf[i]);
		}
		//д��bufferevent�����������
		bufferevent_write(bev, buf, ret);
	}
}

static void
conn_writecb(struct bufferevent* bev, void* user_data)
{

	printf("---call------%s----\n", __FUNCTION__);
	struct evbuffer* output = bufferevent_get_output(bev);
	if (evbuffer_get_length(output) == 0) {
		printf("flushed answer\n");   //��ʾ�Ѿ������д����͵����ݴ����������ˢ�³�ȥ�����͵�Ŀ��Ľ��ն�
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
	struct timeval delay = { 2, 0 };//�����ӳ�ʱ�� 2s

	printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

	event_base_loopexit(base, &delay);//��ʱ2s�˳�
}