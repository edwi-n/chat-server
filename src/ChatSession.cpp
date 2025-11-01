#include "ChatSession.h"
#include "ChatRoom.h"
#include <iostream>
#include <istream>

ChatSession::ChatSession(tcp::socket socket, ChatRoom &room)
    : socket_(std::move(socket)), room_(room) {}

void ChatSession::start()
{
          room_.join(shared_from_this());
          do_read();
}

void ChatSession::deliver(const std::string &msg)
{
          write_msgs_.push_back(msg);
          if (write_msgs_.size() == 1)
          {
                    do_write();
          }
}

void ChatSession::do_read()
{
          asio::async_read_until(socket_, buffer_, "\n",
                                 [this, self = shared_from_this()](std::error_code ec, std::size_t length)
                                 {
                                           if (!ec)
                                           {
                                                     std::istream is(&buffer_);
                                                     std::string message;
                                                     std::getline(is, message);

                                                     // trim '\r' so messages arent overriden when delivered
                                                     if (!message.empty() && message.back() == '\r')
                                                     {
                                                               message.pop_back();
                                                     }

                                                     if (init == false)
                                                     {
                                                               init = true;
                                                               username = message;
                                                     }
                                                     else
                                                     {
                                                               std::string formatted_message = username + ": " + message + "\r\n";
                                                               room_.broadcast(formatted_message, shared_from_this());
                                                     }

                                                     // check for the next messages
                                                     do_read();
                                           }
                                           else
                                           {
                                                     // remove client from the room bcz they left
                                                     room_.leave(shared_from_this());
                                           }
                                 });
}

void ChatSession::do_write()
{
          asio::async_write(socket_,
                            asio::buffer(write_msgs_.front()),
                            [this, self = shared_from_this()](std::error_code ec, std::size_t)
                            {
                                      if (!ec)
                                      {
                                                // use a message queue to overcome async issues
                                                write_msgs_.pop_front();
                                                if (!write_msgs_.empty())
                                                {
                                                          do_write();
                                                }
                                      }
                                      else
                                      {
                                                room_.leave(shared_from_this());
                                      }
                            });
}
