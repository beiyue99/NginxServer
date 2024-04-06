//memory_order_relaxed:
//std::memory_order_relaxed 适用于对顺序不敏感的场景，比如只关心操作的结果而不关心操作的顺序的情况
// 在这种情况下，我们可以使用较轻的内存顺序要求来获得更好的性能。
std::atomic<int> x(0);
void increase_x() {
    for (int i = 0; i < 1000000; ++i) {
        x.fetch_add(1, std::memory_order_relaxed); //将x的值加1，并返回之前的值
    }
}
int main() {
    std::thread t1(increase_x);
    std::thread t2(increase_x);
    t1.join();
    t2.join();
    std::cout << "Final value of x: " << x.load(std::memory_order_relaxed) << std::endl;
    // load:获取变量x的值
    return 0;
}






#include <iostream>
#include <atomic>
#include <thread>

std::atomic<int> data(0);
std::atomic<bool> ready(false);

void producer() {
    data.store(42, std::memory_order_relaxed);
    ready.store(true, std::memory_order_release);
}

void consumer() {
    while (!ready.load(std::memory_order_acquire));
    std::cout << "Data ready: " << data.load(std::memory_order_relaxed) << std::endl;
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();

    return 0;
}
在这个示例中，生产者线程将数据存储到 data 中，并将 ready 设置为 true，采用 std::memory_order_release
内存序确保在写入 data 之后 ready 的写入对消费者可见。而消费者线程采用 std::memory_order_acquire
内存序等待 ready 的值变为 true，以确保在读取 data 之前 ready 的读取对生产者可见。









memory_order_seq_cst:
cpp
Copy code
#include <iostream>
#include <atomic>
#include <thread>

std::atomic<int> counter(0);

void increment_counter() {
    counter.fetch_add(1, std::memory_order_seq_cst);
}

int main() {
    std::thread t1(increment_counter);
    std::thread t2(increment_counter);
    t1.join();
    t2.join();

    std::cout << "Final value of counter: " << counter.load(std::memory_order_seq_cst) << std::endl;

    return 0;
}
在这个示例中，两个线程同时对计数器 counter 进行自增操作，并且使用 std::memory_order_seq_cst 内存序来保证操作的顺序与程序中的顺序一致。





#include <iostream>
#include <atomic>
#include <thread>

std::atomic<int> data(0);

void producer() {
    data.store(42, std::memory_order_relaxed);
}

void consumer() {
    int value = data.load(std::memory_order_relaxed);
    std::cout << "Data value: " << value << std::endl;
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();

    return 0;
}
在这个示例中，生产者线程将数据存储到 data 中，而消费者线程从 data 中加载数据。由于使用了 std::memory_order_relaxed 内存序，因此不保证对 data 的读取和写入顺序。因此，消费者线程可能会看到生产者线程存储的旧值。

这个示例展示了使用内存模型来控制线程之间的同步操作，以确保对共享数据的正确访问。

使用std::atomic的原子操作：
cpp
Copy code
#include <iostream>
#include <atomic>
#include <thread>

std::atomic<int> counter(0);

void increment_counter() {
    for (int i = 0; i < 1000000; ++i) {
        counter.fetch_add(1, std::memory_order_seq_cst);
    }
}

int main() {
    std::thread t1(increment_counter);
    std::thread t2(increment_counter);
    t1.join();
    t2.join();

    std::cout << "Final value of counter: " << counter.load(std::memory_order_seq_cst) << std::endl;

    return 0;
}
在这个示例中，两个线程同时对计数器 counter 进行自增操作，使用了 std::atomic 类型的原子操作 fetch_add() 来确保操作的原子性。这样可以避免竞争条件和数据竞争，确保线程安全。
