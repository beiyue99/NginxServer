主进程初始化信号处理函数，然后创建socket，打开监听端口，
放入监听队列，接下来创建守护进程（fork出的子进程做了主进程）
1,程序执行ngx_master_process_cycle,首先屏蔽信号，防止创建子
进程受到信号干扰，主进程卡在for循环的sigsuspend函数。
2,执行ngx_start_worker_processes创建子进程，父进程直接返回，
子进程调用ngx_worker_process_cycle进入循环
3,循环内部先调用ngx_worker_process_init进行初始化工作，包括
创建线程池，初始化锁、信号量，创建三大线程，创建epoll，创建
连接池（链表），epoll添加监听套间字
4,初始化任务完成后，循环调用ngx_process_events_and_timers
处理网络事件；也就是调epoll_wait等待事件，如果是读写事件，
就调用相应的读写回调函数，监听fd只有读事件。
5,ngx_read_request_handler读回调函数，内部调用recvproc收数据，
收完将数据放入消息队列并激发线程处理
6,再看处理消息队列线程，开始不监测fd写事件，直接发数据，如果
发现写缓冲区满，再检测可写事件，此时是epoll驱动发数据
7,线程入口函数，会被阻塞在条件变量，激活后（有消息处理）调用
threadRecvProcFunc函数拆解数据包，通过消息码调用对应的处理函数



                               
