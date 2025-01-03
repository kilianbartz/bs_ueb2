#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

std::atomic_flag spinlock = ATOMIC_FLAG_INIT;
int next = 1;

void thread_function(int id, int iterations)
{
    int i = 0;
    while (i < iterations)
    {
        // Acquire the spinlock
        while (spinlock.test_and_set(std::memory_order_acquire))
            ; // Busy waiting
        if (next == id)
        {
            // Critical section
            i++;
            // shared_data++;
            // std::cout << "Thread " << id << " incremented shared_data to " << shared_data << std::endl;
            next = 3 - next;
        }
        spinlock.clear(std::memory_order_release);
    }
}

int main()
{
    const int iterations = 1'000'000;
    auto start = std::chrono::high_resolution_clock::now();

    std::thread t1(thread_function, 1, iterations);
    std::thread t2(thread_function, 2, iterations);

    t1.join();
    t2.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> elapsed = end - start;

    // std::cout << "Total time: " << elapsed.count() << " microseconds" << std::endl;
    std::cout << "Average time per iteration: " << elapsed.count() / (2 * iterations) << " microseconds" << std::endl;
    return 0;
}
