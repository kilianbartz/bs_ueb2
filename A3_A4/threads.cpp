#include <iostream>
#include <thread>
#include <zmq.hpp> // ZeroMQ C++ bindings
#include <cstdlib> // For std::atoi
#include <x86intrin.h>

long times = 0;
int iterations_finished = 0;

void worker_thread(zmq::context_t &context, int iterations)
{
    zmq::socket_t receiver(context, zmq::socket_type::pair);
    receiver.connect("inproc://test");

    for (int i = 0; i < iterations; ++i)
    {
        zmq::message_t message;
        receiver.recv(message, zmq::recv_flags::none);

        // Extract the start time from the message
        long start_time = *static_cast<long *>(message.data());
        auto end = __rdtsc();
        times += end - start_time;

        iterations_finished++;
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

    zmq::context_t context(1); // ZeroMQ context with one I/O thread
    zmq::socket_t sender(context, zmq::socket_type::pair);
    sender.bind("inproc://test");

    std::thread worker(worker_thread, std::ref(context), iterations);

    for (int i = 0; i < iterations; ++i)
    {
        long start_time = __rdtsc();
        zmq::message_t message(sizeof(long));
        *static_cast<long *>(message.data()) = start_time; // Embed the start time in the message
        sender.send(message, zmq::send_flags::none);
    }

    worker.join();
    return 0;
}
