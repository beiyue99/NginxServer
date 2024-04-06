

#include <sys/select.h>
struct timeval {
	time_t tv_sec;             // seconds *
	suseconds_t tv_usec;   // microseconds*
};
int select(int nfds, fd_set* readfds, fd_set* writefds,
	fd_set* exceptfds, struct timeval* timeout);
//返回值是检测到的所有满足条件的文件描述符的个数


nfds:委托内核检测的这三个集合中最大的文件描述符 + 1  //最大1024    在Window中这个参数是无效的, 指定为 - 1即可

readfds : 检测读缓冲区，假如换入56789  ，5和6里面有数据，那么传出5和6
writefds : 检测写缓冲区，假如换入78910  ，78910可写，那么传出78910
exceptfds: 如果没有异常，传出NULL；





void FD_CLR(int fd, fd_set * set);// 将文件描述符fd从set集合中删除 = 将fd对应的标志位设置为0

int FD_ISSET(int fd, fd_se * set); // 判断文件描述符fd是否在set集合中 = 读fd对应的标志位到底是0还是1

void FD_SET(int fd, fd_set* set);//将文件描述符fd添加到set集合中==将fd对应的标志位设置为1

void FD_ZERO(fd_set* set);// 将set集合中, 所有文件文件描述符对应的标志位设置为0, 集合中没有添加任何文件描述符




//线性表每个位置对应一个文件描述符，默认每个标志位为0，如果传入线性表集合，就会把该标志位设置为1
//当发现不满足的文件描述符，内核会重新修改该标志位为0，然后通过isset函数可以遍历出满足条件的文件描述符


