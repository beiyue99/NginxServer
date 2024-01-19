无名管道数据只能在同一个方向流动，
数据只能从管道的一端写入，从另一端读出
它没有名字，只能在具有公共祖先的进程（父子，兄弟）之间使用
函数原型： int pipe(int pipefd[2])
功能：创建无名管道

6 int main()
7 {
8     int fds[2];
9     int ret = -1;
10     ret = pipe(fds);
11     if (ret == -1)
12     {
13         perror("pipe");
14         return 1;
15     }
16     //fd[0]用于读，1用于写
17     printf("fd[0]:%d,fd[1]:%d\n", fds[0], fds[1]);
18     close(fds[0]);
19     close(fds[1]);
20     return 0;
    21 }
创建无名管道，返回3和4   （012）被占用





int main()
7 {
8     int fds[2];
9     int ret = -1;
10     int pid = -1;
11     char buf[SIZE] = { 0 };
12     ret = pipe(fds);
13     if (ret == -1)
14     {
15         perror("pipe");
16         return 1;
17     }
18     pid = fork();
19     if (-1 == pid)
20     {
21         perror("fork");
22         return 1;
23     }
24     if (pid == 0)
25     {
26         //子进程，关闭写端
27         close(fds[1]);
28         ret = read(fds[0], buf, SIZE);
29         if (ret < 0)
30         {
31             perror("read");
32             exit(-1);
33         }
34         printf("child pross  buf:%s \n", buf);
35
36         //关闭读端
37         close(fds[0]);
38         exit(0);
39     }
40     //父进程，写管道
41     //关闭读端
42     close(fds[0]);
43     ret = write(fds[1], "ABCDEFGHIJK", 10);
44     if (ret < 0)
45     {
46         perror("write");
47         return 1;
48     }
49     printf("hello parent process write len: %d\n", ret);
50
51     //关闭写端
52     close(fds[1]);
53     return 0;
56 }
//父进程写，子进程读
//无名管道用于有共同祖先的进程





查看管道缓冲区的函数： long fpathconf(int fd, int name);
可以通过name参数查看不同的属性值。 fd : 文件描述符
name : _PC_PIPE_BUF 查看缓冲区大小       _PC_NAME_MAX 查看文件名字字节数上限
返回值：  根据name返回的值意义不同    失败返回 - 1





设置为非阻塞：
//获取原来的flags
int flags = fcntl(fd[0], F_GETFL);
//设置新的flags
flag = O_NONBLOCK;   //flags=flags | O_NONBLOCK;
fcntl(fd[0], F_SETFL, flags);

//如果写端没有关闭，读端为非阻塞，如果没有数据，直接返回-1







7 int main()
8 {
9     int fds[2];
10     int ret = -1;
11     int pid = -1;
12     char buf[SIZE] = { 0 };
13     ret = pipe(fds);
14     if (ret == -1)
15     {
16         perror("pipe");
17         return 1;
18     }
19     pid = fork();
20     if (-1 == pid)
21     {
22         perror("fork");
23         return 1;
24     }
25     if (pid == 0)
26     {
27         //子进程，关闭写端
28         close(fds[1]);
29         printf("读取管道的内容\n");
30         ret = fcntl(fds[0], F_GETFL);
31         ret |= O_NONBLOCK;
32         fcntl(fds[0], F_SETFL, ret); //设置读端为非阻
33         ret = read(fds[0], buf, SIZE);
34         if (ret < 0)
35         {
36             perror("read");
37             exit(-1);
38         }
39         printf("child pross  buf:%s \n", buf);
40
41         //关闭读端
42         close(fds[0]);
43         exit(0);
44     }
45     //父进程，写管道
46     //关闭读端
47     close(fds[0]);
48     sleep(1);
49     ret = write(fds[1], "ABCDEFGHIJK", 10);
50     if (ret < 0)
51     {
52         perror("write");
53         return 1;
54     }
55     printf("hello parent process write len: %d\n", ret);
56
57     //关闭写端
58     close(fds[1]);
59     return 0;
62 }


//设置为非阻塞直接返回



   //无名管道：
     //读的时候
1 写端没有关闭:
没有数据的时候去读会被阻塞，有数据的时候就可以读出
2 写端关闭
会读取全部内容，然后返回0
//写的时候
3：
管道被写满了，写管道进程写的时候会发生阻塞
4：
所有的读端关闭，写管道进程写管道会收到一个信号然后退出。 （写的时候，读端必须开口）







//有名管道注意事项：
一个只读而打开的管道进程会阻塞，直到另一个进程为只写打开该管道
一个只写而打开的管道进程会阻塞，直到另一个进程为只读打开该管道


//有名管道的创建：
int mkfifo(const char* pathname, mode_t mode);
//第一个参数代表创建的管道名字，第二个参数代表文件权限（0666）






8 int main()
9 {
10     int fd = -1;
11     int ret = -1;
12     char buf[SIZE] = { 0 };
13     int i = 0;
14     //以只写的方式打开一个管道文件
15     fd = open("fifoooo", O_WRONLY);
16     if (fd == -1)
17     {
18         perror("open");
19         return 1;
20     }
21     printf("以只写的方式打开一个管道...\n");
22
23     //写管道
24     while (1)
25     {
26         sprintf(buf, "hello world %d", i++);
27         ret = write(fd, buf, strlen(buf));
28         if (ret <= 0)
29         {
30             perror("write");
31             break;
32         }
33         printf("write info:%d\n", ret);   //写入的字节数
34         sleep(1);
35     }
36     //关闭文件
37     close(fd);
38     return 0;
39 }

//.............................................

10 int main()
11 {
12     int fd = -1;
13     int ret = -1;
14     char buf[SIZE] = { 0 };
15     //以只读的方式打开一个管道文件
16     fd = open("fifoooo", O_RDONLY);
17     if (fd == -1)
18     {
19         perror("open");
20         return 1;
21     }
22     printf("以只读的方式打开一个管道....\n");
23     //循环读管道
24     while (1)
25     {
26         ret = read(fd, buf, SIZE);
27         if (ret <= 0)
28         {
29             perror("read");
30             break;
31         }
32         printf("buf:%s\n", buf);
33     }
34     //关闭文件
35     close(fd);
36     return 0;
37 }















: read !head - n 10 test.c




//通过管道实现双方交互对话
1.写管道2，以至于对方可以读到管道2的内容，同时可读取对方写入管道1的内容
10 int main()
11 {
12     int ret = -1;
13     char buf[SIZE] = { 0 };
14     int fdr = -1;
15     int fdw = -1;
16     fdr = open("fifo1", O_RDONLY);
17     if (fdr == -1)
18     {
19         perror("open");
20         return 1;
21     }
22     printf("以只读的方式打开管道1...\n");
23     fdw = open("fifo2", O_WRONLY);
24     if (fdw == -1)
25     {
26         perror("open");
27         return 1;
28     }
29     printf("以只写的方式打开管道2\n");
30     //循环读写
31     while (1)
32     {
33         //读管道1
34         ret = read(fdr, buf, SIZE);
35         if (ret <= 0)
36         {
37             perror("read");
38             break;
39         }
40         printf("read pip1: %s\n", buf);
41         //写管道2
42         fgets(buf, SIZE, stdin);
43         if ('\n' == buf[strlen(buf) - 1])
44         {
45             buf[strlen(buf) - 1] = '\0';
46         }
47         ret = write(fdw, buf, strlen(buf));
48         if (ret <= 0)
49         {
50             perror("write");
51             break;
52         }
53         printf("write pip2:%d\n", ret);
54     }
55     close(fdr);
56     close(fdw);
57     return 0;
58 }




//读2写1

10 int main()
11 {
12     int ret = -1;
13     char buf[SIZE] = { 0 };
14     int fdr = -1;
15     int fdw = -1;
16     fdw = open("fifo1", O_WRONLY);
17     if (fdw == -1)
18     {
19         perror("open");
20         return 1;
21     }
22     printf("以只写的方式打开管道1\n");
23     fdr = open("fifo2", O_RDONLY);
24     if (fdr == -1)
25     {
26         perror("open");
27         return 1;
28     }
29     printf("以只读的方式打开管道2...\n");
30     //循环读写
31     while (1)
32     {
33         //写管道1
34         fgets(buf, SIZE, stdin);
35         if ('\n' == buf[strlen(buf) - 1])
36         {
37             buf[strlen(buf) - 1] = '\0';
38         }
39         ret = write(fdw, buf, strlen(buf));
40         if (ret <= 0)
41         {
42             perror("read");
43             break;
44         }
45         printf("write pip2:%d\n", ret);
46         //读管道2
47         ret = read(fdr, buf, SIZE);
48         printf("read: %s\n", buf);
49         if (ret <= 0)
50         {
51             perror("write");
52             break;
53         }
54     }
55     close(fdr);
56     close(fdw);
57     return 0;
58 }