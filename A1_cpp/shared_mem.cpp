#include <iostream>
#include <atomic>
#include <thread>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <sys/wait.h>

struct SharedMemory
{
    std::atomic_flag spinlock = ATOMIC_FLAG_INIT;
    int shared_data = 0;
};

void process_function(SharedMemory *shm, int id, int iterations)
{
    for (int i = 0; i < iterations; ++i)
    {
        // Acquire the spinlock
        while (shm->spinlock.test_and_set(std::memory_order_acquire))
            ; // Busy waiting

        // Critical section
        // shm->shared_data++;
        // std::cout << "Process " << id << " incremented shared_data to " << shm->shared_data << std::endl;

        // Release the spinlock
        shm->spinlock.clear(std::memory_order_release);
    }
}

int main()
{
    const char *shm_name = "/my_shared_memory";
    const int shm_size = sizeof(SharedMemory);
    const int iterations = 100'000'000;

    // Create shared memory
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, shm_size);
    SharedMemory *shm = (SharedMemory *)mmap(nullptr, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (shm == MAP_FAILED)
    {
        perror("mmap failed");
        return 1;
    }

    // Initialize shared memory
    new (shm) SharedMemory();

    auto start = std::chrono::high_resolution_clock::now();

    // Fork processes
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        process_function(shm, 1, iterations);
    }
    else
    {
        // Parent process
        process_function(shm, 2, iterations);
        wait(nullptr); // Wait for child process
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> elapsed = end - start;

    if (pid > 0)
    { // Parent process
        std::cout << "Total time: " << elapsed.count() << " microseconds" << std::endl;
        std::cout << "Average time per iteration: " << elapsed.count() / (2 * iterations) << " microseconds" << std::endl;

        // Clean up shared memory
        munmap(shm, shm_size);
        shm_unlink(shm_name);
    }

    return 0;
}
