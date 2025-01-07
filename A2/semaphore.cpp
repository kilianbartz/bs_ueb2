#include <iostream>
#include <thread>
#include <semaphore> // For std::binary_semaphore
#include <chrono>
#include <x86intrin.h> // For __rdtsc()

std::binary_semaphore semaphore_main(0);   // Semaphore for the main thread
std::binary_semaphore semaphore_worker(0); // Semaphore for the worker thread
int shared_data = 0;

long start, times = 0;
int iterations_finished = 0;

void thread_function(int iterations)
{
    while (iterations_finished < iterations)
    {
        // Wait for the main thread to signal
        semaphore_worker.acquire();
        if (shared_data > 0)
        {
            auto end = __rdtsc();
            times += end - start;
            iterations_finished++;
            shared_data = 0;
        }
        // Signal the main thread
        semaphore_main.release();
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
    std::thread t1(thread_function, iterations);

    while (iterations_finished < iterations)
    {
        start = __rdtsc();
        shared_data = 42;
        // Signal the worker thread
        semaphore_worker.release();
        // Wait for the worker thread to process
        semaphore_main.acquire();
    }
    t1.join();
    return 0;
}
