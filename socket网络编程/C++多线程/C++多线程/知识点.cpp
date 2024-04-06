#define _CRT_SECURE_NO_WARNINGS 1


//  joinable 判断是否还能detach或join    返回真和假






//defer_lock：前提为没锁。 作用是初始化一个没加锁的互斥量
//比如 unique_lock<mutex> sbguard1(my_mutex1,defer_lock)   如果不加defer_lock默认行为是直接上锁
// 再调用  sbguard1.lock()   该类封装的lock函数会自己解锁，所以后续不需要手动unlock
//该类也封装有unlock函数，以便更灵活的操作数据

//unique_lock的release函数使得与之绑定的互斥量分离，如果此时该互斥量处于锁定状态，需要后续手动unlock

//std::unique_lock::try_lock
//std::try_to_lock
// 
// std::mutex mtx;





//{
//    std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
//    if (lock.owns_lock()) {
//        // 如果互斥量成功锁定，执行这个代码块
//        // ... 在这里执行一些需要保护的代码 ...
//    }
//    else {
//        // 如果互斥量没有被锁定，执行这个代码块
//        // ... 在这里处理互斥量被锁定的情况 ...
//    }
//} // 如果互斥量被锁定，它会在这里被自动解锁






//std::mutex mtx;
//
//{
//    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
//    // 在这里，mtx没有被锁定
//
//    if (lock.try_lock()) {
//        // 如果能立即锁定互斥量，就会执行这个代码块
//        // ... 在这里执行一些需要保护的代码 ...
//    }
//    else {
//        // 如果不能立即锁定互斥量，就会执行这个代码块
//        // ... 在这里处理互斥量被锁定的情况 ...
//    }
//} // 如果互斥量被锁定，它会在这里被自动解锁
