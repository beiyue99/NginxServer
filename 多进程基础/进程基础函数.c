




getpid(); 获取进程号   getppid();获取父进程号  


  
  
  
  
 //fork函数在子进程中返回0，在父进程中返回子进程的pid

  
  
  
fork()遵从“读时共享，写时拷贝”  如果父进程修改变量的值，会拷贝一份修改，子进程再读取这个变量还是原来的值
  
  
  
  
  

  

      //raise函数  给自己当前进程发送信号。等价于kill(getpid(),sig)
      int raise(int sig);     成功返回0，失败返回一个非零值


      int mian()
  {
      int i = 1;
      while (1)
      {
          printf("do working %d\n", i);
          if (i == 4)
          {
              //给自己发送编号为15的信号,默认行为是终止进程
              raise(SIGTERM);
          }
          i++;
          sleep(1);
      }
      return 0;
  }


  void abort(void);
  给自己发送异常终止信号6（SIGABRT），并产生core文件，等价于kill（getpid(), SIGABRT);





