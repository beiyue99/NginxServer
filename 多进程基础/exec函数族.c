//进程替换： 通过exec函数族，他们的功能都相似
execl和execlp需要掌握，其余的了解

进程调用一种exec函数时，该进程安全由新进程替换，从main函数开始执行。并未创建新进程，只是用另一个新进程替换了当前进程的正文，数据，堆和栈。




1：
int execl(const char* pathname, const char* arg, ...
/* (char  *) NULL */);
第一个参数为可执行文件的绝对路径和相对路径，第二个参数为可执行文件的名字，然后是可执行文件的参数，最后一个是NULL，与execlp类似，
除了第一个参数不同，功能相同

execl("/bin/ls", "ls", "-l", "/home", NULL);



2：
int execlp(const char* file, const char* arg, ...
/* (char  *) NULL */);

5 int main()
6 {
    7     printf("hello world\n");
    8     execlp("ls", "ls", "-l", "/home", NULL); //等价于执行ls -l /home //第一个是file文件名，后面是arg0到argn。约定arg0为文件名，argn为NULL
    9     printf("hello 222222222\n");
    10     return 0;
    11 }
第9行不会执行，因为运行到第八行，进程被替换






3：
int execv(const char* pathname, char* const argv[]);
中心代码段：
char* argv[] = { "ls","-l","/home",NULL);
printf("hello world");
execv("/bin/ls",argv);
printf("hello 22222222\n");
return 0;


4：
       int execvp(const char* file, char* const argv[]);

       中心代码段：
       char* argv[] = {"ls","-l","/home",NULL);
       printf("hello world");
       execvp("ls",argv);
       printf("hello 22222222\n");
       return 0;