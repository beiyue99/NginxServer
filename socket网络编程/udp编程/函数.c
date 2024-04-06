tcp像打电话，建立一次链接可以一直通讯，udp像寄信，不用建立连接，每次都要携带目的地信息交流

TCP它使用了一系列的机制来提高数据传输的可靠性，
包括丢包检测和重传机制。以下是TCP如何处理这些问题的：

序列号和确认应答（ACK）：TCP为每个数据包分配一个唯一的序列号，并且在接收到数据包后，
接收方会返回一个确认应答（ACK），告诉发送方它已经接收到了那个序列号的数据包。
如果发送方在一定时间内没有收到某个数据包的ACK，它会认为那个数据包已经丢失，并且会重新发送那个数据包。

滑动窗口：TCP使用滑动窗口机制来进行流量控制和拥塞控制。滑动窗口可以限制未确认的数据包数量，
这样如果网络环境不好，或者接收方处理能力不足，TCP就可以减小发送速度，防止数据包丢失。

拥塞控制：当网络中出现拥塞时，TCP有机制可以减小发送速率以避免丢包。如果发送方连续收到几个重复的ACK，
或者超时没有收到ACK，它就会认为网络出现了拥塞，然后减小滑动窗口大小，降低发送速度。




sendto() : 发送数据到一个特定的地址
sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientaddr, clientlen);
sendto()函数向指定的地址发送数据。buffer是要发送的数据，strlen(buffer)是数据的长度，
clientaddr是客户端的地址，clientlen是客户端地址的长度。




recvfrom() : 从一个特定的地址接收数据。
recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientaddr, &clientlen);
recvfrom()函数从指定的地址接收数据。buffer是用于存储接收到的数据的缓冲区，sizeof(buffer)是缓冲区的大小，
clientaddr是客户端的地址，clientlen是客户端地址的长度。此函数返回接收到的字节数。如果出错，此函数返回 - 1。





int main() {
    FILE* fp = fopen("nonexistentfile.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // Other code...
    return 0;
}
如果文件不存在，这段代码可能会输出"Failed to open file: No such file or directory"。

至于strerror(errno)和perror哪个更方便，这主要取决于你的具体需求。perror会自动输出一个消息，这个消息包含你提供的前缀和系统的错误描述，所以在许多情况下，perror可能更简单易用。然而，strerror提供了更多的灵活性，因为它只是返回一个描述错误的字符串，而不是直接输出它。这意味着你可以将这个字符串存储起来，稍后输出，或者以其他方式处理。所以如果你需要更多的灵活性，strerror可能会更有用。

请注意，由于strerror返回的字符串可能指向一个静态缓冲区，所以在多线程环境中，它可能不是线程安全的。在这种情况下，你可能需要使用strerror_r函数，这个函数是线程安全的。





std::cerr默认是不进行缓冲的，也就是说，当你向std::cerr写入数据时，数据会立即输出到屏幕。



FILE* fp = fopen("nonexistentfile.txt", "r");
if (fp == NULL) {
    perror("Failed to open file");
}
如果文件不存在，这段代码可能会输出"Failed to open file: No such file or directory"。




int main() {
    FILE* fp = fopen("nonexistentfile.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // Other code...
    return 0;
}
如果文件不存在，这段代码可能会输出"Failed to open file: No such file or directory"。
由于strerror返回的字符串可能指向一个静态缓冲区，所以在多线程环境中，它可能不是线程安全的。
在这种情况下，你可能需要使用strerror_r函数，这个函数是线程安全的



stderr是C语言中的一个预定义的文件指针，它表示标准错误输出流。这个流通常被用于输出错误信息或者诊断信息。
stderr的特点是它默认是无缓冲的，这意味着当你向stderr写入数据时，数据会立即输出到屏幕，而不是先存储在缓冲区。
这使得stderr非常适合于错误消息，因为当错误发生时，你通常希望立即知道，而不是等待缓冲区被刷新




socket API原本是为网络通讯设计的，但后来在socket的框架上发展出一种IPC机制，就是UNIX Domain Socket。
但是UNIX Domain Socket用于IPC更有效率：只是将应用层数据从一个进程拷贝到另一个进程。
UNIX Domain Socket是全双工的，API接口语义丰富，相比其它IPC机制有明显的优越性，目前已成为使用最广泛的IPC机制，
比如X Window服务器和GUI程序之间就是通过UNIXDomain Socket通讯的。




UNIX Domain Socket与网络socket编程最明显的不同在于地址格式不同，用结构体sockaddr_un表示，
网络编程的socket地址是IP地址加端口号，而UNIX Domain Socket的地址是一个socket类型的文件在文件系统中的路径，
这个socket文件由bind()调用创建，如果调用bind()时该文件已存在，则bind()错误返回。
对比网络套接字地址结构和本地套接字地址结构：
struct sockaddr_in {
    __kernel_sa_family_t sin_family; 			/* Address family */  	地址结构类型
        __be16 sin_port;					 	/* Port number */		端口号
        struct in_addr sin_addr;					/* Internet address */	IP地址
};
struct sockaddr_un {
    __kernel_sa_family_t sun_family; 		/* AF_UNIX */			地址结构类型
        char sun_path[UNIX_PATH_MAX]; 		/* pathname */		socket文件名(含路径)
};




如果sun_path数组没有完全填充（例如，它是一个字符串，而且没有使用完整的UNIX_PATH_MAX个字符），
那么使用sizeof(struct sockaddr_un)可能会得到一个比实际有效数据大很多的值。在这种情况下，
你可以使用offsetof宏和strlen函数来计算出实际有效数据所占用的字节数量，
方法就是offsetof(struct sockaddr_un, sun_path) + strlen(sun_path)。这将返回sun_family成员的大小，
以及sun_path数组中实际有效字符串的长度，从而得到一个更精确的结果。




//客户端路径乱码问题：
UNIX域套接字（也被称为本地套接字）允许在同一台主机上的两个进程进行双向通信。它是一种进程间通信机制，
与网络套接字类似，但只能在同一台机器上使用。
与网络套接字不同的是，UNIX域套接字不使用IP地址和端口来识别进程，而是使用一个文件系统路径名
你创建一个服务端UNIX域套接字并绑定到一个路径名时，这个路径名代表的是服务端的标识。
当客户端要连接到服务端时，它会使用这个路径名来找到服务端
当客户端调用connect()函数时，如果没有提前调用bind()给自己的套接字分配路径名，系统会自动为其分配一个临时路径。
这个路径名通常是在 / tmp目录下的一个随机文件名，这就是为什么你看到的客户端路径名看起来像是一些随机字符。


如果客户端没有显式地调用bind()函数来绑定一个端口，那么操作系统会为其隐式地选择一个临时端口。
在这个场景中，因为我们是用nc命令行工具作为客户端，所以确实发生了隐式绑定。


运行你的服务器程序：. / server。这将创建和监听sock.s套接字。
在另一个终端窗口中，使用nc - U sock.s连接到这个服务器。
nc（netcat）是一个用于处理TCP和UDP连接的工具。这是一个非常强大的工具，
可以用于测试网络连接，编写脚本，调试，甚至在网络上发送文件。
- U选项是用来指定要使用Unix域套接字，而不是TCP / UDP套接字。
sock.s是你的Unix域套接字的路径名。