#include <iostream>
#include <cstdlib>
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

                    // run the asio event loop
                    io_context.run();
          }
          catch (std::exception &e)
          {
                    std::cerr << "Exception: " << e.what() << "\n";
          }

          return 0;
}
