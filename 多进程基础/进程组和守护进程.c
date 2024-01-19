  进程组和守护进程



pid_t getpgrp(void);
功能:获取当前进程的进程组ID


pid_t getpgid(pid_t pid);
功能:获取指定进程的进程组ID
参数 :
pid:进程号, 如果pid = O, 那么该函数作用和getpgrp一样



int setpgid(pid_t pid, pid_t pgid);
将参1对应的进程, 加入参2对应的进程组中



会话是一个或者多个进程组的集合
创建会话调用的进程不能是进程组组长进程，也就是说组长不能变会长
一个进程创建会话后，这个进程将成为组长进程，也就是说，它将同时成为组长和会长




pid_t getsid(pid_t pid);
功能:获取进程所属的会话ID


		



		pid_t setsid(void);
		创建一个会话, 并以自己的ID设置进程组ID, 同时也是新会话的ID。
		调用了setsid函数的进程,既是新的会长, 也是新的组长。
		
			


			只有会话领导者才能调用 setsid() 函数创建新的会话。这是因为 setsid() 的目的是创建一个新的会话，
			并确保调用它的进程成为新会话的领导者。通常情况下，这个进程是一个不与终端相关联的孤立进程。

			进程组领导者可以调用 setsid() 函数成功，但这并不会创建一个新的会话。如果进程组领导者调用 setsid()，
			则该进程仍然保持在原有的会话中，而不会创建新的会话。

			因此，为了创建一个新的会话，确实需要由一个非会话领导者的进程调用 setsid()。一种常见的做法是先 fork() 一个子进程，
			然后在子进程中调用 setsid()，从而确保子进程成为新的会话领导者










#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
			//创建守护进程
			int main()
		{
			int ret = -1;
			pid_t  pid = -1;
			//1.创建子进程，父进程退出(必须)
			pid = fork();
			if (-1 == pid)
			{
				perror("fork");
				return 1;
			}
			if (pid > 0)
			{
				exit(0);
			}

			//2.创建新的会话,使进程安全脱离控制终端,此时
			//即使退出终端，此进程依然还存在
			pid = setsid();
			if (pid == -1)
			{
				perror("setsid");
				return 1;
			}


			//3.改变当前的工作目录
			ret = chdir("/");

			//4.设置权限掩码
			//umask函数用于设置文件创建时的默认权限掩码。 这是因为创建新文件时，
			// 文件的权限会受到当前进程的umask值的影响。通过调用umask(0)，
			// 将文件创建的默认权限掩码设置为0，意味着新创建的文件将具有完全的权限（读、写、执行），而不受umask的限制。
			//这对于确保守护进程在创建文件时有足够的权限是很重要的，尤其是当它需要写入日志文件等操作时。
			umask(0);
			//5.关闭文件描述符
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);
			//6.执行核心任务
			 //每隔一秒钟输出当前的时间到/tmp/txt.log文件中
			while (1)
			{
				system("date >> /tmp/txt.log");
				sleep(1);
			}

			return 0;
		}
