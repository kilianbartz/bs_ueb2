#include <zmq.hpp>
#include <iostream>

int main()
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::rep);
    socket.bind("ipc:///tmp/test");

    while (true)
    {
        zmq::message_t request;
        socket.recv(request, zmq::recv_flags::none);
        zmq::message_t reply(request.size());
        memcpy(reply.data(), request.data(), request.size());
        socket.send(reply, zmq::send_flags::none);
    }

    return 0;
}
