#include <iostream>
#include <atomic>
#include <chrono>
#include <x86intrin.h> // For __rdtsc()
#include <sys/mman.h>  // For mmap
#include <unistd.h>    // For fork, getpid
#include <cstring>     // For memset
#include <sys/wait.h>  // For wait

struct SharedMemory
{
    std::atomic_flag spinlock = ATOMIC_FLAG_INIT;
    int shared_data = 0;
    long start = 0;
    long times = 0;
    int iterations_finished = 0;
};

void process_function(SharedMemory *shm, int iterations)
{
    while (shm->iterations_finished < iterations)
    {
        // Acquire the spinlock
        while (shm->spinlock.test_and_set(std::memory_order_acquire))
            ;

        if (shm->shared_data > 0)
        {
            auto end = __rdtsc();
            shm->times += end - shm->start;
            shm->iterations_finished++;
            shm->shared_data = 0;
        }

        // Release the spinlock
        shm->spinlock.clear(std::memory_order_release);
    }

    std::cout << "Average time per iteration: " << shm->times / iterations << " cycles" << std::endl;
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
    // Create shared memory
    SharedMemory *shm = static_cast<SharedMemory *>(mmap(
        nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0));

    if (shm == MAP_FAILED)
    {
        perror("mmap");
        return 1;
    }

    // Initialize shared memory
    memset(shm, 0, sizeof(SharedMemory));
    shm->spinlock.clear(std::memory_order_release);

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        munmap(shm, sizeof(SharedMemory));
        return 1;
    }

    if (pid == 0)
    {
        // Child process
        process_function(shm, iterations);
        munmap(shm, sizeof(SharedMemory));
        return 0;
    }
    else
    {
        // Parent process
        while (shm->iterations_finished < iterations)
        {
            // Acquire the spinlock
            while (shm->spinlock.test_and_set(std::memory_order_acquire))
                ;

            shm->start = __rdtsc();
            shm->shared_data = 42;

            // Release the spinlock
            shm->spinlock.clear(std::memory_order_release);
        }

        // Wait for the child process to finish
        wait(nullptr);

        munmap(shm, sizeof(SharedMemory));
    }

    return 0;
}
