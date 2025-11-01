#ifndef CHAT_SESSION_H
#define CHAT_SESSION_H

#include <memory>
#include <string>
#include <deque>
#include <asio.hpp>

// forward declaration
class ChatRoom;

using asio::ip::tcp;

// represents a single chat session
class ChatSession : public std::enable_shared_from_this<ChatSession>
{
public:
          ChatSession(tcp::socket socket, ChatRoom &room);
          void start();
          void deliver(const std::string &msg); // this can get called by ChatRoom to send a message

private:
          // async loops
          void do_read();
          void do_write();

          tcp::socket socket_;
          ChatRoom &room_;
          asio::streambuf buffer_;
          std::deque<std::string> write_msgs_; // message queue
          std::string username = "";
          bool init = false;
};

#endif
