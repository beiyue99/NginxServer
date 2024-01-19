wait和waitpid功能一样，都是等待一个子进程结束，回收这个子进程的资源。区别在于，wait函数会阻塞，waitpid可以设置不阻塞，还可以指定等待某个子进程结束

wait函数返回值为已经结束子进程的进程id，失败返回 - 1，调用该函数的进程会阻塞，直到子进程退出或者收到一个不可忽视的信号才被唤醒继续向下执行，
若调用该函数的进程没有子进程，那么该函数立即返回。若子进程已经结束，该函数也会立即返回，并且回收已经结束的子进程的资源。
pid_t wait(int* status)
status：进程退出时的状态信息。如果status的值不是NULL，就会把退出时子进程的状态存入其中。直接使用这个值是没有意义的，还需要借助相关宏取出它代表的信息











下面看 waitpid
pid_t waitpid(pit_t pid, int* status, int options);

参数pid的类型:
1  pid > 0 ： 等待进程id = pid的子进程
2  pid = 0 :  等待同一个进程组中的任何子进程，如果子进程已经加入了别的进程组，便不会等待它
3  pid = -1 : 等待任一子进程，此时和wait函数作用一样
4  pid < -1 : 等待指定进程组中的任何子进程，这个进程组的id等于pid的绝对值

	参数options提供了一些额外的选项来控制waitpid（）
	O : 同wait()，阻塞父进程，等待子进程退出
	WNOHANG： 没有任何已经结束的子进程，则立即返回，也就是不等待子进程
	WUNTRACED : 如果进程暂停了，此函数马上返回，并且不予以理会子进程的结束状态（极少用到）


	返回值比较复杂，有三个情况：
	1： 当正常返回的时候，它返回回收的子进程的进程号
	2： 如果设置了选项WNOHANG，而调用函数时发现没有已退出的子进程可等待，返回0
	3： 如果出错返回 - 1，并设置errno。如当pid所对应的子进程不存在，或者此进程存在，但不是调用进程的子进程，就会出错




int main()
{
    int ret = -1;
     int status = -1;
     int pid = -1;
     pid = fork();
     if (pid == -1)
     {
         perror("fork");
         return 1;
     }
     if (pid == 0)
     {
         for (int i = 0; i < 5; i++)
         {
             printf("child progress %d is  doing things! %d\n", getpid(), i);
             sleep(1);
         }
         exit(10);
     }
     printf("父进程等待子进程退出\n");
 //  ret=wait(&status);
 //  ret=waitpid(-1,&status,0);跟wait效果一样
 //  ret=waitpid(-1,&status,WNOHANG);不等待子进程，也就是不阻塞

     if (ret == -1)
     {
         perror("wait");
         return 1;
     }
     printf("父进程回收了子进程的资源\n");
     if (WIFEXITED(status))
     {
         printf("子进程正常退出，状态码为%d\n", WEXITSTATUS(status));
     }
     else if (WIFSIGNALED(status))
     {
         printf("子进程被信号%d杀死了...\n", WTERMSIG(status));
     }
     else if (WIFSTOPPED(status))
     {
         printf("子进程被信号%d暂停...\n", WSTOPSIG(status));
     }
     return 0;
 }