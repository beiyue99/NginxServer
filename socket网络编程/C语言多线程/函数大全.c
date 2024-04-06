


pthread_create()
//函数原型：int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
函数参数：
thread : 指向 pthread_t 类型的指针，用于返回新创建线程的 ID。
attr : 指向 pthread_attr_t 类型的指针，用于指定新创建线程的属性。如果不需要指定属性，可以传入 NULL。
start_routine : 新线程的入口函数。
arg : 传递给新线程入口函数的参数。
函数返回值：
成功：返回 0。
失败：返回错误码。


void* thread_function(void* arg) {
    // 线程的执行逻辑
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_attr_t attr;

    // 初始化线程属性
    pthread_attr_init(&attr);

    // 设置线程为分离状态
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // 创建线程，将属性作为第二个参数传递
    pthread_create(&thread, &attr, thread_function, NULL);

    // 销毁线程属性
    pthread_attr_destroy(&attr);

    // 不需要调用 pthread_join

    return 0;
}
设置线程为分离状态后，主线程不能再对该线程进行资源回收或等待其结束，因为该线程将在结束时自动释放资源



pthread_join()
函数原型：int pthread_join(pthread_t thread, void** retval);
函数作用：等待一个线程结束，并获取线程返回值。
函数参数：
thread : 等待结束的线程 ID。
retval : 指向 void 类型指针的指针，用于返回线程的返回值。
函数返回值：
成功：返回 0。
失败：返回错误码。






pthread_exit()
函数原型：void pthread_exit(void* retval);
函数作用：使当前线程退出，并返回一个指定的返回值。
函数参数：
retval : 线程的返回值。
函数返回值：无。








pthread_mutex_init()
函数原型：int pthread_mutex_init(pthread_mutex_t* mutex, const pthread_mutexattr_t* attr);
函数作用：初始化一个互斥锁。
函数参数：
mutex : 指向 pthread_mutex_t 类型的指针，用于指定要初始化的互斥锁。
attr : 指向 pthread_mutexattr_t 类型的指针，用于指定互斥锁的属性。如果不需要指定属性，可以传入 NULL。
函数返回值：
成功：返回 0。
失败：返回错误码。







pthread_mutex_lock()
函数原型：int pthread_mutex_lock(pthread_mutex_t * mutex);
函数作用：加锁一个互斥锁。
函数参数：
mutex : 指向 pthread_mutex_t 类型的指针，用于指定要加锁的互斥锁。
函数返回值：
成功：返回 0。
失败：返回错误码。








pthread_mutex_unlock()
函数原型：int pthread_mutex_unlock(pthread_mutex_t * mutex);
函数作用：解锁一个互斥锁。
函数参数：
mutex : 指向 pthread_mutex_t 类型的指针，用于指定要解锁的互斥锁。
函数返回值：
成功：返回 0。
失败：返回错误码。







pthread_cond_init()
函数原型：int pthread_cond_init(pthread_cond_t * cond, const pthread_condattr_t * attr);
函数作用：初始化一个条件变量。
函数参数：
cond : 指向 pthread_cond_t 类型的指针，用于指定要初始化的条件变量。
attr : 指向 pthread_condattr_t 类型的指针，用于指定条件变量的属性。如果不需要指定属性，可以
传入 NULL。
函数返回值：
成功：返回 0。
失败：返回错误码。







pthread_cond_wait()
函数原型：int pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex);
函数作用：等待一个条件变量的信号，并且解锁一个互斥锁，使得其他线程可以访问被保护的共享资源。
函数参数：
cond : 指向 pthread_cond_t 类型的指针，用于指定要等待的条件变量。
mutex : 指向 pthread_mutex_t 类型的指针，用于指定要解锁的互斥锁。
函数返回值：
成功：返回 0。
失败：返回错误码。






pthread_cond_signal()
函数原型：int pthread_cond_signal(pthread_cond_t * cond);
函数作用：发送一个信号给等待一个条件变量的线程。
函数参数：
cond : 指向 pthread_cond_t 类型的指针，用于指定要发送信号的条件变量。
函数返回值：
成功：返回 0。
失败：返回错误码。





pthread_t pthread_self(void);
功能:获取线程号。 此函数总会成功





int pthread_equal(pthread_t t1, pthread_t t2);
功能:
判断线程号t1和t2是否相等。为了方便移植, 尽量使用函数来比较线程ID
返回值   相等返回非零，不相等返回零

