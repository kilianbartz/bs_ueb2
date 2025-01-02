#include <zmq.hpp>
#include <thread>
#include <iostream>
#include <chrono>

void server_thread(zmq::context_t &context)
{
    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.bind("inproc://test");

    while (true)
    {
        zmq::message_t request;
        socket.recv(request, zmq::recv_flags::none);
        zmq::message_t reply(request.size());
        memcpy(reply.data(), request.data(), request.size());
        socket.send(reply, zmq::send_flags::none);
    }
}

int main()
{
    zmq::context_t context(1);

    // Start server thread
    std::thread server(server_thread, std::ref(context));

    // Client socket
    zmq::socket_t socket(context, zmq::socket_type::req);
    socket.connect("inproc://test");

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

    std::cout << "Average latency (inproc): " << elapsed.count() / num_messages << " µs" << std::endl;

    server.join();
    return 0;
}
