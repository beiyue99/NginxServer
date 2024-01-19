//储存映射函数的使用：
#inlcude<sys / mman.h>
void* mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset);
//功能： 将一个文件或者对象映射进内存
//参数： 
addr:  指定映射的起始地址。通常为NULL
length : 映射到内存的文件长度
prot : 映射区的保护方式，其中常用的：
1.读： PROT_READ
2.写： PROT_WRITE
3.读写： PROT_READ | PROT_WROTE
flags : 映射区的特性，一般是：
1. MAP_SHARED：写入映射区的数据会复制回文件，且允许其它映射该文件的进程共享
2. MAP_PEIVATE：会产生一个映射区的复制，对此映射区所做的修改不会写会原文件

fd : 所要映射的文件的文件描述符
offset : 文件开始处的偏移量，必须是4k的整数倍，通常为0，表示从头开始映射
返回值： 成功返回映射区首地址，失败返回MAP_FAILED宏



解除关联用 munmap函数  int munmap(void* addr, size_t length);
功能： 释放内存映射区
addr : 使用mmap函数创建的内存映射区的首地址
length : 映射区的大小
成功返回0   失败返回 - 1







10 int main()
11 {
    12     int fd = -1;
    13     int ret = -1;
    14     void* addr = NULL;
    15     //已读写的方式打开一个文件
        16     fd = open("txt", O_RDWR);
    17     if (fd == -1)
        18     {
        19         perror("open");
        20         return 1;
        21     }
    22     //将文件映射到内存
        23     addr = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    24     if (addr == MAP_FAILED)
        25     {
        26         perror("mmap");
        27         return 1;
        28     }
    29     printf("文件储存映射ok...\n");
    30     //关闭文件
        31     close(fd);
    32     //写入映射区
        33     memcpy(addr, "1234567890", 10);
    34     munmap(addr, 1024);
    35     return 0;
    36 }

//将txt原来的前10个字节覆盖

注意事项：
1.创建映射区的过程中，隐含着一次对映射文件的读操作
2.当MAP_SHARED时，映射区的权限必须小于原文件的权限
3.映射区的释放与文件关闭无关。只要映射建立成功，文件可以立即关闭
4.映射文件大小不能为0
5.创建映射区出错概率高，最好检查返回值




//父子进程通过储存映射实现进程通讯
1 #include<sys/wait.h>
2 #include<stdio.h>
3 #include<string.h>
4 #include<sys/types.h>
5 #include<sys/stat.h>
6 #include<sys/mman.h>
7 #include<stdlib.h>
8 #include<fcntl.h>
9 #include<unistd.h>
10
int main()
{
    int fd = -1;
    int ret = -1;
    pid_t pid = -1;
    void* addr = NULL;
    //已读写的方式打开一个文件
    fd = open("txt", O_RDWR);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }
    //将文件映射到内存
    addr = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }
    printf("文件储存映射ok...\n");
    //关闭文件
    close(fd);
    //创建一个子进程
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return 1;
    }
    if (pid == 0)
    {
        memcpy(addr, "1234567890", 10);
    }
    else
    {
        wait(NULL);   //阻止父程进程，直到其任何孩子都完成为止
        printf("addr:%s\n", (char*)addr);
    }
    munmap(addr, 1024);
    return 0;
}








//非父子进程之间利用储存映射进行通讯


//读进程
11 int main()
12 {
    13     int fd = -1;
    14     int ret = -1;
    15     pid_t pid = -1;
    16     void* addr = NULL;
    17     //以读写的方式打开一个文件
        18     fd = open("txt", O_RDWR);
    19     if (fd == -1)
        20     {
        21         perror("open");
        22         return 1;
        23     }
    24     //将文件映射到内存
        25     addr = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    26     if (addr == MAP_FAILED)
        27     {
        28         perror("mmap");
        29         return 1;
        30     }
    31     printf("文件储存映射ok...\n");
    32     //关闭文件
        33     close(fd);
    34     //读存储映射区
        35     printf("addr,%s\n", (char*)addr);
    36     munmap(addr, 1024);
    37     return 0;
    38 }




//写进程
11 int main()
12 {
    13     int fd = -1;
    14     int ret = -1;
    15     pid_t pid = -1;
    16     void* addr = NULL;
    17     //已读写的方式打开一个文件
        18     fd = open("txt", O_RDWR);
    19     if (fd == -1)
        20     {
        21         perror("open");
        22         return 1;
        23     }
    24     //将文件映射到内存
        25     addr = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    26     if (addr == MAP_FAILED)
        27     {
        28         perror("mmap");
        29         return 1;
        30     }
    31     printf("文件储存映射ok...\n");
    32     //关闭文件
        33     close(fd);
    34     //写存储映射区
        35     memcpy(addr, "1234567890", 10);
    36     munmap(addr, 1024);
    37     return 0;
    38 }









//使用匿名映射实现父子间进程通信


13 int main()
14 {
    15     int ret = -1;
    16     pid_t pid = -1;
    17     void* addr = NULL;
    18     //1.创建匿名映射
        19     addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    20     if (MAP_FAILED == addr)
        21     {
        22         perror("mmap");
        23         return 1;
        24     }
    25     //2.创建子进程
        26     pid = fork();
    27     if (pid == -1)
        28     {
        29         perror("fork");
        30         munmap(addr, 4096);
        31         return 1;
        32     }
    33     //3.父子间通信
        34     if (pid == 0)
        35     {
        36         //子进程写
            37         memcpy(addr, "1234567890", 10);
        38     }
    39     else
        40     {
        41         //父进程读
            42         wait(NULL);
        43         printf("parent process %s\n", (char*)addr);
        44     }
    45     //4.断开映射
        46     munmap(addr, 4096);
    47     return 0;
    48 }