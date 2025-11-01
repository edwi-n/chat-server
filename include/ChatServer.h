#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <asio.hpp>
#include "ChatRoom.h"

using asio::ip::tcp;

// listens for connections and creates sessions
class ChatServer
{
public:
          ChatServer(asio::io_context &io_context, const tcp::endpoint &endpoint);

private:
          // async loop to check for connections
          void do_accept();

          tcp::acceptor acceptor_;
          ChatRoom room_; // single room for all the clients
};

#endif
