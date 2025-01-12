#include <iostream>
#include <thread>
#include <semaphore> // For std::binary_semaphore
#include <chrono>
#include <x86intrin.h> // For __rdtsc()

std::binary_semaphore semaphore_server(0); // Semaphore for the server
std::binary_semaphore semaphore_client(0); // Semaphore for the client thread
int shared_data = 0;

long start, times = 0;
int iterations_finished = 0;

void thread_function(int iterations)
{
    while (iterations_finished < iterations)
    {
        // Wait for the server to signal
        semaphore_client.acquire();
        if (shared_data > 0)
        {
            auto end = __rdtsc();
            times += end - start;
            iterations_finished++;
            shared_data = 0;
        }
        // Signal the server
        semaphore_server.release();
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
        semaphore_server.acquire();
        start = __rdtsc();
        shared_data = 42;
        // Signal the client thread
        semaphore_client.release();
        // Wait for the client thread to process
    }
    t1.join();
    return 0;
}
