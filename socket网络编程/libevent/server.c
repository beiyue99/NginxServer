#include <stdio.h>
#include <unistd.h>
#include <event.h>
#include "wrap.h"
#include <string.h>

#define MAX_FD 1024

struct event* events[MAX_FD];

void cfdcb(int cfd, short event, void* arg) 
{
    char buf[1500] = "";
    struct event* ev = events[cfd];
    int n = Read(cfd, buf, sizeof(buf));
    if (n <= 0) 
    {
        if (n < 0) 
        {
            perror("Read error");
        }
        else 
        {
            printf("Client closed connection\n");
        }
        close(cfd); // �رտͻ�������
        event_del(ev); // ɾ���¼�
        event_free(ev); // �ͷ��¼�
        events[cfd] = NULL;
        return; // �˳��ص�����
    }
    else 
    {
        printf("recive from client:%s\n", buf);
        int ret = Write(cfd, buf, n); // ��������һ�� echo �������������յ������ݻش����ͻ���
        if (ret < 0)
        {
            printf("Write error: %s\n", strerror(-ret)); // ע�������� -ret ����ȡ������
            close(cfd); // �رտͻ�������
            event_del(ev); // ɾ���¼�
            event_free(ev); // �ͷ��¼�
            events[cfd] = NULL;
            return; // �˳��ص�����
        }

    }
}

void lfdcb(int lfd, short event, void* arg)
{
    struct event_base* base = (struct event_base*)arg;

    // ��ȡ�µ�cfd
    int cfd = Accept(lfd, NULL, NULL);
    if (cfd < 0)
    {
        perror("Accept error");
        return;
    }
    printf("cfd is %d\n", cfd);
    // ��cfd����
    struct event* ev = event_new(base, cfd, EV_READ | EV_PERSIST, cfdcb, NULL);

    if (!ev) 
    {
        perror("Could not create event for cfd");
        close(cfd); // �رտͻ�������
        return;
    }
    events[cfd] = ev;
    event_add(ev, NULL);
}

int main(int argc, char* argv[])
{
    // �����׽��ֲ���
    int lfd = tcp4bind(8888, NULL);
    if (lfd < 0) 
    {
        perror("tcp4bind error");
        return 1;
    }
    if (Listen(lfd, SOMAXCONN)<0)
    {
        perror("Listen error");
        return 1;
    }
    // ���� event base ���ڵ�
    struct event_base* base = event_base_new();
    if (!base)
    {
        perror("Could not create event base");
        close(lfd);
        return 1;
    }

    // ��ʼ�� lfd �����ڵ�
    struct event* ev = event_new(base, lfd, EV_READ | EV_PERSIST, lfdcb, base);
//lfd��base�ֱ����Ϊ�ص�����lfdcb�ĵ�һ�������͵�����������
//���ص������ĵڶ�������event�����Libevent����ݾ��屻�������¼����������á�
    if (!ev)
    {
        perror("Could not create event for lfd");
        close(lfd);
        event_base_free(base);
        return 1;
    }

    // ����
    event_add(ev, NULL);

    // ѭ������
    event_base_dispatch(base); // ����

    // ��β
    for (int i = 0; i < MAX_FD; i++) 
    {
        if (events[i]) 
        {
            event_free(events[i]);
            events[i] = NULL;
        }
    }
    close(lfd);
    event_base_free(base);

    return 0;
}



//Ϊʲô�����ȵ�һ�κã�  ��һ��Ϊʲôû������ȫ��events�����¼��أ�������ô�ͷŵģ�
//��Դ�����ڶ��δ���ʹ��ȫ�� events �������洢ÿ���ͻ��˵� event�������ڿͻ��˶Ͽ�����ʱ�����Է�����ҵ����ͷŶ�Ӧ�� event��
//����һ�δ��벢û�����Եĵط����洢�͹�����Щ event��������ĳЩ����������Դй©��
//
//������ĵڶ������⣬��һ�δ����У�event ���� bufferevent �д����͹���ġ��� bufferevent ���� EOF ���ߴ����ͷ�ʱ
//���� echo_event_cb �����е��� bufferevent_free����������ص� event Ҳ�ᱻ�Զ��ͷš�
//
//Ȼ������ʹ��ˣ��ڶ��δ�����ʹ��ȫ�� events ������� event �ķ�ʽ���������ŵ�ġ���ʹ���¼����������ڸ��������Ϳɿأ�
//���Ա����ڸ�������¿��ܳ��ֵ���Դй©�����⣬������������Ҫʱ�ṩ�����л event �ķ��ʣ������ڷ������ر�ʱ��Ҫ�ͷ����е� event��