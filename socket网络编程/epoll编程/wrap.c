//上面的例子不仅功能简单, 而且简单到几乎没有什么错误处理, 我们知道, 系统调用不能保证每次都成功,
//必须进行出错处理, 这样一方面可以保证程序逻辑正常, 另一方面可以迅速得到故障信息。
//为使错误处理的代码不影响主程序的可读性, 我们把与socket相关的一些系统函数加上错误处理代码包装成
//新的函数, 做成一个模块wrap.c:
#include "wrap.h"
void perr_exit(const char* s)
{
	perror(s);
	exit(1);
}
int Accept(int fd, struct sockaddr* sa, socklen_t* salenptr)
{
	int n;
again:
	if ((n = accept(fd, sa, salenptr))	< 0) 
	{
		if ((errno == ECONNABORTED) || (errno == EINTR))
			goto again;
		else
			perr_exit("accept error");
	}
	return n;
}




int Bind(int fd, const struct sockaddr* sa, socklen_t salen)
{
	int n;
	if ((n = bind(fd, sa, salen))< 0)
		perr_exit("bind error");
	return n;
}




int tcp4bind(short port, const char* IP)
{
	struct sockaddr_in serv_addr;
	int lfd = Socket(AF_INET, SOCK_STREAM, 0);
	// 设置端口复用
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	memset(&serv_addr, 0, sizeof(serv_addr));
	if (IP == NULL)
	{
		//如果这样使用0.0.0.0,任意ip将可以连接
		serv_addr.sin_addr.s_addr = INADDR_ANY;
	}
	else 
	{
		if (inet_pton(AF_INET, IP, &serv_addr.sin_addr.s_addr) <= 0) 
			//使用inet_pton函数将IP参数的字符串形式转换为网络字节序的IPv4地址，并将结果存储在s_addr成员变量中
		{
			perror(IP);//转换失败
			exit(1);
		}
	}
	serv_addr.sin_port = htons(port);
	//将传入的port参数由主机字节序转换为网络字节序，并将结果存储在sin_port成员变量中。	
	serv_addr.sin_addr.s_addr = INADDR_ANY;


	if (bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	return lfd;
}









int Connect(int fd, const struct sockaddr* sa, socklen_t salen)
{
	int n;
	if ((n = connect(fd, sa, salen))< 0)
		perr_exit("connect error");
	return n;
}



int Listen(int fd, int backlog)
{
	int n;
	if ((n = listen(fd, backlog))< 0)
		perr_exit("listen error");
	return n;
}




int Socket(int family, int type, int protocol)
{
	int n;
	if ((n = socket(family, type, protocol))< 0)
		perr_exit("socket error");
	return n;
}



ssize_t Read(int fd, void* ptr, size_t nbytes)
{
	ssize_t n;
again:
	if ((n = read(fd, ptr, nbytes))== -1) {
		if (errno == EINTR)
			goto again;
		else
			return -1;
	}
	return n;
}



ssize_t Write(int fd, const void* ptr, size_t nbytes)
{
	ssize_t n;
again:
	if ((n = write(fd, ptr, nbytes))== -1) 
	{
		if (errno == EINTR)  //EINTR 代表一个系统调用因为接收到信号而被中断，
			//如果一个函数在执行时返回 EINTR，那通常意味着这个函数可能需要被重新执行。
			goto again;
		else
			return -errno;
	}
	return n;
}



int Close(int fd)
{
	int n;
	if ((	n = close(fd))== -1)
		perr_exit("close error");
	return n;
}
ssize_t Readn(int fd, void* vptr, size_t n)
{
	size_t nleft;
	ssize_t nread;
	char* ptr;
	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft))< 0) {
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else if (nread == 0)
			break;
		nleft -= nread;
		ptr += nread;
	}
	return n - nleft;
}
ssize_t Writen(int fd, const void* vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char* ptr;
	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft))<= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return n;
}




//这个函数用于从文件描述符fd中读取一个字符到ptr。为了提高效率，
//这个函数会一次读取多个字符（最多100个），然后逐个返回。当所有字符都返回后，它会再次尝试读取。

/*这个函数在读取时可能会被中断（比如因为接收到了一个信号），所以它使用了goto语句在被中断时重新尝试读取。
虽然在许多情况下使用goto是不推荐的，但在处理错误和中断的时候，使用goto可以使代码更加清晰*/ 
ssize_t my_read(int fd, char* ptr)
{
	static int read_cnt;
	static char* read_ptr;
	static char read_buf[100];
	if (read_cnt <= 0)
	{
	again:
		if ((read_cnt = read(fd, read_buf, sizeof(read_buf)))< 0) 
		{
			if (errno == EINTR)

	/*			errno是一个全局变量，用于在C和C++中报告错误。当你调用一些可能会失败的函数（比如read）时，
				如果发生错误，这些函数通常会设置errno的值来指示具体的错误类型。

				EINTR是errno的一个可能值，代表了“Interrupted system call”（被中断的系统调用）。
				当你的程序正在执行一个系统调用（比如read）时，如果接收到一个可以被捕获的信号，
				那么系统调用可能会被中断，而不是正常完成。当这种情况发生时，系统调用会返回 - 1，并且设置errno为EINTR。*/
			goto again;
			return -1;
		}
		else if (read_cnt == 0)
			return 0;
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}



//从文件描述符fd中读取一行数据，直到遇到换行符\n或读取到的字符数达到maxlen为止。
//这个函数会将读取到的数据存储到vptr指向的内存空间中，并在结束时添加一个空字符作为结束标志。
ssize_t Readline(int fd, void* vptr, size_t maxlen)
{
	ssize_t n, rc;
	char c, * ptr;
	ptr = vptr;
	for (n = 1; n < maxlen; n++) 
	{
		if ((rc = my_read(fd, &c))== 1) 
		{
			*ptr++ = c;
			if (c == '\n')
			break;
		}
		else if (rc == 0) 
		{
			*ptr = 0;
			return n - 1;
		}
		else
			return -1;
	}
	*ptr = 0;
	return n;
}