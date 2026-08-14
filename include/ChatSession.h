#ifndef CHAT_SESSION_H
#define CHAT_SESSION_H

#include <memory>
#include <string>
#include <array>
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
    void process_incoming();
    bool push_outgoing(const std::string &msg);
    bool pop_outgoing(std::string &msg);
    bool try_pop_incoming_message(std::string &msg);

    tcp::socket socket_;
    ChatRoom &room_;

    static constexpr std::size_t incoming_capacity = 4096;
    static constexpr std::size_t outgoing_capacity = 64;

    std::array<char, incoming_capacity> incoming_buffer_{};
    std::size_t incoming_head_ = 0;
    std::size_t incoming_size_ = 0;

    std::array<std::string, outgoing_capacity> write_msgs_{};
    std::size_t write_head_ = 0;
    std::size_t write_size_ = 0;
    bool writing_ = false;

    std::string username = "";
    bool init = false;
    bool joined_ = false;
};

#endif
