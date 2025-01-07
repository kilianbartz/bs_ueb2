#include <iostream>
#include <zmq.hpp>     // ZeroMQ C++ bindings
#include <x86intrin.h> // For __rdtsc()
#include <cstdlib>     // For std::atoi
#include <thread>      // For sleep

int main(int argc, char *argv[])
{
    // Check if the number of iterations is provided as a command-line argument
    if (argc < 2 || argc > 3)
    {
        std::cerr << "Usage: " << argv[0] << " <number_of_iterations> [socket_address]" << std::endl;
        return 1;
    }

    // Convert the argument to an integer
    int iterations = std::atoi(argv[1]);
    if (iterations <= 0)
    {
        std::cerr << "Error: The number of iterations must be a positive integer." << std::endl;
        return 1;
    }
    std::string socket_address = (argc == 3) ? argv[2] : "ipc:///tmp/test";

    zmq::context_t context(1); // ZeroMQ context with one I/O thread
    zmq::socket_t sender(context, zmq::socket_type::pair);
    sender.bind(socket_address);

    for (int i = 0; i < iterations; ++i)
    {
        long start_time = __rdtsc();
        zmq::message_t message(sizeof(long));
        *static_cast<long *>(message.data()) = start_time; // Embed the start time in the message
        sender.send(message, zmq::send_flags::none);

        // Optional: Add a small delay to simulate real-world conditions
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    std::cout << "Server: Sent " << iterations << " messages." << std::endl;
    return 0;
}
