#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool turn = true; // Gibt an, welcher Thread an der Reihe ist (true = Thread 1, false = Thread 2)

void thread_function(int id, int iterations)
{
    for (int i = 0; i < iterations; ++i)
    {
        std::unique_lock<std::mutex> lock(mtx);

        // Warten, bis der aktuelle Thread an der Reihe ist
        cv.wait(lock, [id]()
                { return (turn && id == 1) || (!turn && id == 2); });

        // Wechsel der Reihenfolge
        turn = !turn;

        // Anderen Thread benachrichtigen
        cv.notify_one();
    }
}

int main()
{
    const int iterations = 1000;

    // Zeitmessung starten
    auto start = std::chrono::high_resolution_clock::now();

    // Zwei Threads starten
    std::thread t1(thread_function, 1, iterations);
    std::thread t2(thread_function, 2, iterations);

    // Auf Threads warten
    t1.join();
    t2.join();

    // Zeitmessung beenden
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> elapsed = end - start;

    std::cout << "Total time: " << elapsed.count() << " microseconds" << std::endl;
    std::cout << "Average time per iteration: " << elapsed.count() / (2 * iterations) << " microseconds" << std::endl;

    return 0;
}
