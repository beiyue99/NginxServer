




//主机字节序的ip地址是字符串,网络字节序ip地址是整形   


//小端转大端
int inet_pton(int af, const char* src, void* dst);
af:地址族(ip地址的家族包括ipv4和ipv6)协议
src : 传入参数, 对应要转换的点分十进制的ip地址 : 192.168.1.100
dst : 传出参数, 函数调用完成, 转换得到的大端整形IP被写入到这块内存中


//将大端的整形数,转换为小端的点分十进制的IP地址          
const char* inet_ntop(int af, const void* src, char* dst, socklen_t size);
参数:
size : 修饰dst参数的, 标记dst指向的内存中最多可以存储多少个字节







int socket(int domain, int type, int protocol);

domain：指定套接字使用的协议族或地址族，可以是以下常用值之一：
AF_INET：IPv4 网络协议。
AF_INET6：IPv6 网络协议。
AF_UNIX：UNIX 域协议，用于本地进程间通信。

type：指定套接字的类型，可以是以下常用值之一：
SOCK_STREAM：提供可靠的、面向连接的字节流，使用 TCP 协议。
SOCK_DGRAM：提供不可靠的、无连接的数据报服务，使用 UDP 协议。
SOCK_RAW：提供原始网络协议存取。

protocol：指定使用的协议，一般为 0，表示根据 domain 和 type 的取值选择默认协议。




函数 bind() 用于将一个套接字（socket）绑定到一个特定的地址和端口。
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
//通常创建in4 或者in6的结构体传参，方便初始化
//绑定时 ，端口或者ip必须是大端的
sockfd：要绑定的套接字的文件描述符。
addr：指向要绑定的地址信息的结构体指针，通常是 struct sockaddr 类型的指针，需要进行类型转换。
addrlen：addr 结构体的大小，可以使用 sizeof(struct sockaddr) 来获取。


struct sockaddr {
    sa_family_t sa_family;  // 地址族（协议族）
    char sa_data[14];       // 地址数据
};

sa_family：表示地址族或协议族，可以是 AF_INET（IPv4）、AF_INET6（IPv6）或其他支持的地址族常量。
sa_data：存储地址数据的字段，具体内容和长度取决于地址族的不同


在实际使用中，通常使用更具体的地址结构体类型
//struct sockaddr_in（IPv4 地址结构体）的定义：
struct sockaddr_in {
    sa_family_t sin_family;  // 地址族（协议族）
    in_port_t sin_port;      // 16 位端口号
    struct in_addr sin_addr; // IPv4 地址
    char sin_zero[8];        // 用于补齐，通常设置为全 0
};


sin_family：表示地址族或协议族，设置为 AF_INET 表示 IPv4 地址族。
sin_port：16 位端口号，使用网络字节序存储。
sin_addr：存储 IPv4 地址的字段，类型为 struct in_addr，用于存储 32 位 IPv4 地址。
sin_zero：用于补齐字段，通常设置为全 0。
其中，struct in_addr 是一个用于存储 IPv4 地址的结构体。它的定义如下：

struct in_addr {
    in_addr_t s_addr;  // 存储 IPv4 地址的无符号整数
};
struct in_addr 中的 s_addr 是一个无符号整数，用于存储 32 位的 IPv4 地址。













//struct sockaddr_in6 的定义
struct sockaddr_in6 {
    sa_family_t sin6_family;     // 地址族（协议族）
    in_port_t sin6_port;         // 16 位端口号
    uint32_t sin6_flowinfo;      // 流标识
    struct in6_addr sin6_addr;   // IPv6 地址
    uint32_t sin6_scope_id;      // 作用域标识
};
sin6_family：表示地址族或协议族，设置为 AF_INET6 表示 IPv6 地址族。
sin6_port：16 位端口号，使用网络字节序存储。
sin6_flowinfo：流标识，用于区分同一源地址和目标地址之间不同的流。
sin6_addr：存储 IPv6 地址的字段，类型为 struct in6_addr，用于存储 128 位 IPv6 地址。
sin6_scope_id：作用域标识，用于指示地址的范围（例如，接口的索引）。
其中，struct in6_addr 是一个用于存储 IPv6 地址的结构体。它的定义如下：

struct in6_addr {
    unsigned char s6_addr[16];  // 存储 IPv6 地址的字节数组
};
struct in6_addr 中的 s6_addr 是一个长度为 16 的无符号字符数组，用于存储 IPv6 地址的 16 字节表示。











accept() 函数会阻塞程序的执行，直到有新的连接请求到达，并且从等待连接队列中取出一个连接请求进行处理。
如果等待连接队列为空，则程序会一直等待，直到有连接请求到达为止。





connect()函数是一个阻塞函数，即当调用该函数时，程序会一直等待连接建立或发生错误。
如果你希望在建立连接过程中设置超时时间，可以使用select()或poll()等函数实现非阻塞操作。





接收数据
ssize_t read(int sockfd, void* buf, size_t size);
ssize_t recv(int sockfd, void* buf, size_t size,int flags);
参数：     
sockfd:用于通信的文件描述符, accept()函数的返回值
buf : 指向一块有效内存, 用于存储接收是数据
size : 参数buf指向的内存的容量
fags : 特殊的属性, 一般不使用, 指定为0
返回值
大于0 : 实际接收的字节数
等于0 : 对方断开了连接
 - 1 : 接收数据失败了
如果连接没有断开, 接收端接收不到数据, 接收数据的函数会阻塞等待数据到达, 数据到达后函数解除阻塞, 开始接收数据, 当
发送端断开连接, 接收端无法接收到任何数据, 但是这时候就不会阻塞了, 函数直接返回0。



发送数据的函数
ssize_t write(int fd, const void* buf, size_t len);
ssize_t send(int fd const void* buf, size_t len, int flags)
·参数
fd : 通信的文件描述符, accept函数的返回值
buf : 传入参数, 要发送的字符串
len : 要发送的字符串的长度
lags : 特殊的属性, 一般不使用, 指定为0





    //int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
    //sockfd：套接字描述符，指定要设置选项的套接字。
    //level：选项所属的协议层或套接字层。SOL_SOCKET（通用套接字选项）和特定协议的级别，如 IPPROTO_TCP（TCP协议选项）。
    //optname：要设置的选项名称。这是一个整数常量，表示特定选项的标识符。
    //optval：指向包含选项值的缓冲区的指针。
    //optlen：选项值的长度。
//这段代码中的 setsockopt() 函数用于设置服务器套接字的选项。特别是，它使用了 SO_REUSEADDR 和 SO_REUSEPORT 选项。
/*  SO_REUSEADDR 和 SO_REUSEPORT 是套接字选项，用于控制套接字的行为。
它们通常用于在关闭套接字后立即重新启动服务器，而无需等待一段时间以使套接字地址可再次使用。
具体作用如下：
SO_REUSEADDR 选项允许套接字地址（IP地址和端口号）在释放后立即重用。这意味着即使之前的套接字连接尚未完全关闭，也可以立即绑定到相同的地址上。
SO_REUSEPORT 选项允许多个套接字绑定到相同的地址和端口上。这对于实现负载均衡和多进程 / 线程并发处理请求的服务器非常有用。*/





//将ip和端口存到结构体需要转化为大端，htons() 和 htonl() 是用于在(小端)和(大端)之间进行转换的函数。

int main(int argc, char* argv[])
{
	char buf[4] = { 192,168,1,2 };
	int num = * (int*)buf;
	int sum = htonl(num);   //小端转大端
	unsigned char* p= &sum;
	printf("%d%d%d%d\n", *p, *(p + 1), *(p + 2), *(p + 3));
	unsigned short a = 0x0102;
	unsigned short b =  htons(a);
	printf("%x\n", b);   //0201
	return 0;
}


int main(int argc, char* argv[])
{
	unsigned char buf[4] = { 1,1,168,192 };
	int  num = * (int*)buf;
	int sum = ntohl(num);  //大端转小端
	unsigned char* p= (unsigned char*)& sum;
	printf("%d%d%d%d\n", *p, *(p + 1), *(p + 2), *(p + 3);
	return 0;
}

