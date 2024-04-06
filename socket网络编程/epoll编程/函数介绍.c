



epoll_ctl函数：

原型：int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);    
参数：
op：操作类型，可以是EPOLL_CTL_ADD（添加事件）、EPOLL_CTL_MOD（修改事件）或EPOLL_CTL_DEL（删除事件）。
fd：要监听的文件描述符。
event：指向epoll_event结构体的指针，用于描述要监听的事件类型。
epoll_event结构体定义如下：
struct epoll_event{
    _uint32_t events;  // 表示感兴趣的事件类型
    epoll_data_t data;  // 用户数据，可以是文件描述符等辅助信息
};

typedef union epoll_data 
//这个是储存用户数据,当对应的事件触发，内核会传出一个epoll_event，里面的date就是我们传入的date
{
    void* ptr;
    int fd;
    int uint32_t u32
        uint64_t   u64;

}epoll_data_t;




epoll_wait函数：

原型：int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);
功能：等待事件的发生，并将发生的事件返回给调用者。
参数：
events：指向epoll_event结构体数组的指针，用于接收发生的事件。
maxevents：events数组的大小，表示最多可以接收多少个事件。
timeout：等待的超时时间，单位是毫秒， - 1表示永久等待，0表示立即返回。
返回值：返回发生事件的文件描述符的数量，如果超时返回0，如果出错返回 - 1，并设置errno来指示错误类型。







水平模式（Level - Triggered）：
当文件描述符准备好进行 I / O 操作时，会触发 epoll 事件。
在水平模式下，如果文件描述符处于就绪状态，epoll_wait() 函数将一直返回该文件描述符的就绪事件，
直到该事件被处理（即读取完数据或写入完数据）。
水平模式适用于非阻塞 I / O，对于每个就绪的文件描述符，需要循环读取或写入数据直到没有数据可读或可写。

边缘模式（Edge - Triggered）：
当文件描述符的状态发生变化时，会触发 epoll 事件。
在边缘模式下，epoll_wait() 函数只会返回文件描述符状态变化的事件，不会重复返回已经就绪的事件。
边缘模式适用于阻塞 I / O，可以使用非阻塞模式或多线程来处理就绪事件。
处理完一个事件后，需要重新调用 epoll_wait() 获取下一个事件。
一个读事件，如果一次性没有读完，后续不会继续触发事件，除非又有了新的读事件


如果使用 epoll 的边沿模式进行读事件的检测，
有新数据达到只会通知一次，那么必须要保证得到通知后将数据全部从读缓冲区中读出。那么，应该如何读这些数据呢？
方式 1：准备一块特别大的内存，用于存储从读缓冲区中读出的数据，但是这种方式有很大的弊端：
内存的大小没有办法界定，太大浪费内存，太小又不够用

方式 2：循环接收数据
这样做也是有弊端的，因为套接字操作默认是阻塞的，当读缓冲区数据被读完之后，
读操作就阻塞了也就是调用的 read() / recv() 函数被阻塞了，当前进程 / 线程被阻塞之后就无法处理其他操作了。
要解决阻塞问题，就需要将套接字默认的阻塞行为修改为非阻塞，需要使用 fcntl() 函数进行处理：
// 设置完成之后, 读写都变成了非阻塞模式
int flag = fcntl(cfd, F_GETFL);  //获取文件描述符属性
flag |= O_NONBLOCK;             //设置非阻塞
fcntl(cfd, F_SETFL, flag);


在非阻塞模式下，循环地将读缓冲区数据读到本地内存中，当缓冲区数据被读完了，
调用的 read() / recv() 函数还会继续从缓冲区中读数据，此时函数调用就失败了，返回 - 1，
对应的全局变量 errno 值为 EAGAIN 或者 EWOULDBLOCK 如果打印错误信息会得到如下的信息：Resource temporarily unavailable


// 非阻塞模式下recv() / read()函数返回值 len == -1
int len = recv(curfd, buf, sizeof(buf), 0);
if (len == -1)
{
    if (errno == EAGAIN | EWOULDBLOCK)
    {
        printf("数据读完了...\n");
    }
    else
    {
        perror("recv");
        exit(0);
    }
}



int scandir(const char* dirp,struct dirent*** namelist,int(*filter)(const struct dirent*)
    ,int(*compar)(const struct dirent**, const struct dirent**));

第二个参数是一个三级指针，假如struct dirent* mylist指向指针数组的首元素的地址，每个元素都是dirent*类型，namelist就是mylist的地址。
mylist指向一个指针数组，每次读取到文件，就开辟一个内存块储存这个文件内容(指的不是文件本身，是他的信息)
每个文件内容都被存到指针数组里面的结构体指针里面：
struct dirent {
    ino_t          d_ino;       /* inode编号 */
    off_t          d_off;       /* 到下一个目录项的偏移 */
    unsigned short d_reclen;    /* 这个记录的长度 */
    unsigned char  d_type;      /* 文件类型；不是所有文件系统类型都支持 */
    char           d_name[256]; /* 文件名 */
};


dirp：这是指向目录路径的指针，我们希望从中读取条目。
namelist：这是指向指针的指针，将指向匹配项数组。这个数组由函数动态分配。当你用完这个数组后，你应该释放它。
filter：这是一个函数指针，用于过滤目录中的条目。如果这个参数为NULL，所有的条目都会返回。
compar：这是一个函数指针，用于比较两个目录条目。如果这个参数为NULL，条目可能按未定义顺序返回。
这个函数在使用时通常配合alphasort()函数一起使用，alphasort()是一个预定义的比较函数，用于按字母顺序对目录项进行排序。
返回值是读取到文件的个数


int main(void) {
    struct dirent** namelist;
    int n;

    n = scandir(".", &namelist, NULL, alphasort);
    if (n < 0) {
        perror("scandir");
    }
    else {
        while (n--) {
            printf("%s\n", namelist[n]->d_name);
            free(namelist[n]);
        }
        free(namelist);
    }
}    
在这个例子中，我们扫描当前目录（"."），不使用过滤器（所以我们传递NULL作为第三个参数），
并使用alphasort()来对结果进行排序。然后，我们打印出所有的文件和目录名，然后释放动态分配的内存。




readdir()函数是用于读取目录中的条目的。
struct dirent* readdir(DIR* dirp);   dirp：这是一个指向DIR类型的指针，通常是由opendir()函数返回的。
    



int main(void)
{
    DIR* dir;
    struct dirent* entry;

    dir = opendir(".");
    if (dir == NULL)
    {
        perror("opendir");
        return 1;
    }
    while ((entry = readdir(dir)) != NULL)  
    {
        printf("%s\n", entry->d_name);
    }
    closedir(dir);
    return 0;
}

我们首先打开当前目录（"."），然后使用readdir()在一个循环中读取所有的条目。
每次循环，我们都打印出条目的名称。最后，我们使用closedir()关闭目录。














