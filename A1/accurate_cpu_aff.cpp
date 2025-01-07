#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <x86intrin.h> // For __rdtsc()
#include <pthread.h>   // For setting thread affinity on Linux
#include <sched.h>     // For CPU affinity macros

std::atomic_flag spinlock = ATOMIC_FLAG_INIT;
int shared_data = 0;

long start, times = 0;
int iterations_finished = 0;

void set_cpu_affinity(int core_id)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = pthread_self();
    if (pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset) != 0)
    {
        std::cerr << "Error setting CPU affinity for core " << core_id << std::endl;
    }
}

void thread_function(int iterations)
{
    // Set this thread to run on core 1
    set_cpu_affinity(1);

    while (iterations_finished < iterations)
    {
        while (spinlock.test_and_set(std::memory_order_acquire))
            ;
        if (shared_data > 0)
        {
            auto end = __rdtsc();
            times += end - start;
            iterations_finished++;
            shared_data = 0;
        }
        spinlock.clear(std::memory_order_release);
    }
    std::cout << "Average time per iteration: " << times / iterations << " cycles" << std::endl;
}

int main(int argc, char *argv[])
{
    // Check if the number of iterations is provided as a command-line argument
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <number_of_iterations>" << std::endl;
        return 1;
    }

    // Convert the argument to an integer
    int iterations = std::atoi(argv[1]);
    if (iterations <= 0)
    {
        std::cerr << "Error: The number of iterations must be a positive integer." << std::endl;
        return 1;
    }
    // Set the main thread to run on core 0
    set_cpu_affinity(0);

    spinlock.test_and_set(std::memory_order_acquire);
    start = __rdtsc();
    spinlock.clear(std::memory_order_release);
    std::thread t1(thread_function, iterations);

    while (iterations_finished < iterations)
    {
        while (spinlock.test_and_set(std::memory_order_acquire))
            ;
        start = __rdtsc();
        shared_data = 42;
        spinlock.clear(std::memory_order_release);
    }
    t1.join();
    return 0;
}
