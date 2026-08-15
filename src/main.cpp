#include <iostream>
#include <cstdlib>
#include <thread>
#include <vector>
#include <asio.hpp>
#include "ChatServer.h"

using asio::ip::tcp;

int main(int argc, char *argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: chat_server <port>\n";
            return 1;
        }

        asio::io_context io_context;

        // get the port and create the endpoint
        std::uint16_t port = std::atoi(argv[1]);
        tcp::endpoint endpoint(tcp::v4(), port);

        // create the server
        ChatServer server(io_context, endpoint);

        std::cout << "Chat server started on port " << port << "..." << std::endl;

        constexpr std::size_t worker_count = 4;
        std::vector<std::thread> workers;
        workers.reserve(worker_count);

        for (std::size_t i = 0; i < worker_count; ++i)
        {
            workers.emplace_back([&io_context]()
                                 { io_context.run(); });
        }

        // run the asio event loop on the main thread too
        io_context.run();

        for (auto &worker : workers)
        {
            worker.join();
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
