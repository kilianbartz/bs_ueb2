#include <iostream>
#include <zmq.hpp> // ZeroMQ C++ bindings
#include <cstdlib> // For std::atoi
#include <x86intrin.h>

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
    zmq::socket_t receiver(context, zmq::socket_type::pair);
    receiver.connect(socket_address);

    long times = 0;

    for (int i = 0; i < iterations; ++i)
    {
        zmq::message_t message;
        receiver.recv(message, zmq::recv_flags::none);

        // Extract the start time from the message
        long start_time = *static_cast<long *>(message.data());
        auto end = __rdtsc();
        times += end - start_time;
    }

    std::cout << "Client: Average time per iteration: " << times / iterations << " cycles" << std::endl;
    return 0;
}
