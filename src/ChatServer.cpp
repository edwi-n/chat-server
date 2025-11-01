#include "ChatServer.h"
#include "ChatSession.h"
#include <memory>

ChatServer::ChatServer(asio::io_context &io_context, const tcp::endpoint &endpoint)
    : acceptor_(io_context, endpoint)
{
          do_accept();
}

void ChatServer::do_accept()
{
          acceptor_.async_accept(
              [this](std::error_code ec, tcp::socket socket)
              {
                        if (!ec)
                        {
                                  // create a new session
                                  std::make_shared<ChatSession>(std::move(socket), room_)->start();
                        }
                        // continue listening for more sesions
                        do_accept();
              });
}
