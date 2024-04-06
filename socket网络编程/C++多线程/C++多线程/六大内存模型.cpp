//memory_order_relaxed:
//std::memory_order_relaxed �����ڶ�˳�����еĳ���������ֻ���Ĳ����Ľ���������Ĳ�����˳������
// ����������£����ǿ���ʹ�ý�����ڴ�˳��Ҫ������ø��õ����ܡ�
std::atomic<int> x(0);
void increase_x() {
    for (int i = 0; i < 1000000; ++i) {
        x.fetch_add(1, std::memory_order_relaxed); //��x��ֵ��1��������֮ǰ��ֵ
    }
}
int main() {
    std::thread t1(increase_x);
    std::thread t2(increase_x);
    t1.join();
    t2.join();
    std::cout << "Final value of x: " << x.load(std::memory_order_relaxed) << std::endl;
    // load:��ȡ����x��ֵ
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
�����ʾ���У��������߳̽����ݴ洢�� data �У����� ready ����Ϊ true������ std::memory_order_release
�ڴ���ȷ����д�� data ֮�� ready ��д��������߿ɼ������������̲߳��� std::memory_order_acquire
�ڴ���ȴ� ready ��ֵ��Ϊ true����ȷ���ڶ�ȡ data ֮ǰ ready �Ķ�ȡ�������߿ɼ���









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
�����ʾ���У������߳�ͬʱ�Լ����� counter ������������������ʹ�� std::memory_order_seq_cst �ڴ�������֤������˳��������е�˳��һ�¡�





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
�����ʾ���У��������߳̽����ݴ洢�� data �У����������̴߳� data �м������ݡ�����ʹ���� std::memory_order_relaxed �ڴ�����˲���֤�� data �Ķ�ȡ��д��˳����ˣ��������߳̿��ܻῴ���������̴߳洢�ľ�ֵ��

���ʾ��չʾ��ʹ���ڴ�ģ���������߳�֮���ͬ����������ȷ���Թ������ݵ���ȷ���ʡ�

ʹ��std::atomic��ԭ�Ӳ�����
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
�����ʾ���У������߳�ͬʱ�Լ����� counter ��������������ʹ���� std::atomic ���͵�ԭ�Ӳ��� fetch_add() ��ȷ��������ԭ���ԡ��������Ա��⾺�����������ݾ�����ȷ���̰߳�ȫ��
