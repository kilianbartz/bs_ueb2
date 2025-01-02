#include <zmq.hpp>
#include <iostream>
#include <chrono>

int main()
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::req);
    socket.connect("ipc:///tmp/test");

    const int num_messages = 100'000'000;
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_messages; ++i)
    {
        zmq::message_t request(5);
        memcpy(request.data(), "ping", 5);
        socket.send(request, zmq::send_flags::none);

        zmq::message_t reply;
        socket.recv(reply, zmq::recv_flags::none);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::micro> elapsed = end - start;

    std::cout << "Average latency (ipc): " << elapsed.count() / num_messages << " µs" << std::endl;

    return 0;
}
