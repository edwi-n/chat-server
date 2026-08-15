#include "ChatSession.h"
#include "ChatRoom.h"
#include <iostream>
#include <algorithm>

ChatSession::ChatSession(tcp::socket socket, ChatRoom &room)
    : socket_(std::move(socket)), strand_(asio::make_strand(socket_.get_executor())), room_(room) {}

void ChatSession::start()
{
    asio::post(strand_, [self = shared_from_this()]
               { self->do_read(); });
}

void ChatSession::deliver(const std::string &msg)
{
    asio::post(strand_, [self = shared_from_this(), msg]
               {
                   if (!self->push_outgoing(msg))
                   {
                       if (self->joined_)
                       {
                           self->room_.leave(self);
                           self->joined_ = false;
                       }
                       return;
                   }

                   if (!self->writing_)
                   {
                       self->do_write();
                   } });
}

void ChatSession::process_incoming()
{
    std::string message;

    while (try_pop_incoming_message(message))
    {
        if (init == false)
        {
            if (message.empty())
            {
                continue;
            }

            init = true;
            username = message;

            if (!joined_)
            {
                room_.join(shared_from_this());
                joined_ = true;
            }
        }
        else
        {
            std::string formatted_message = username + ": " + message + "\r\n";
            room_.broadcast(formatted_message, shared_from_this());
        }
    }
}

void ChatSession::do_read()
{
    if (incoming_size_ == incoming_capacity)
    {
        if (joined_)
        {
            room_.leave(shared_from_this());
            joined_ = false;
        }
        return;
    }

    const std::size_t write_index = (incoming_head_ + incoming_size_) % incoming_capacity;
    const std::size_t contiguous_space = std::min(incoming_capacity - incoming_size_, incoming_capacity - write_index);

    socket_.async_read_some(
        asio::buffer(&incoming_buffer_[write_index], contiguous_space),
        asio::bind_executor(
            strand_,
            [this, self = shared_from_this()](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    incoming_size_ += length;
                    process_incoming();
                    do_read();
                }
                else
                {
                    // remove client from the room bcz they left
                    if (joined_)
                    {
                        room_.leave(shared_from_this());
                        joined_ = false;
                    }
                }
            }));
}

void ChatSession::do_write()
{
    if (write_size_ == 0)
    {
        writing_ = false;
        return;
    }

    writing_ = true;

    asio::async_write(socket_,
                      asio::buffer(write_msgs_[write_head_]),
                      asio::bind_executor(
                          strand_,
                          [this, self = shared_from_this()](std::error_code ec, std::size_t)
                          {
                              if (!ec)
                              {
                                  std::string ignored;
                                  pop_outgoing(ignored);

                                  if (write_size_ > 0)
                                  {
                                      do_write();
                                  }
                                  else
                                  {
                                      writing_ = false;
                                  }
                              }
                              else
                              {
                                  writing_ = false;
                                  if (joined_)
                                  {
                                      room_.leave(shared_from_this());
                                      joined_ = false;
                                  }
                              }
                          }));
}

bool ChatSession::push_outgoing(const std::string &msg)
{
    if (write_size_ == outgoing_capacity)
    {
        return false;
    }

    const std::size_t write_index = (write_head_ + write_size_) % outgoing_capacity;
    write_msgs_[write_index] = msg;
    ++write_size_;
    return true;
}

bool ChatSession::pop_outgoing(std::string &msg)
{
    if (write_size_ == 0)
    {
        return false;
    }

    msg = std::move(write_msgs_[write_head_]);
    write_msgs_[write_head_].clear();
    write_head_ = (write_head_ + 1) % outgoing_capacity;
    --write_size_;
    return true;
}

bool ChatSession::try_pop_incoming_message(std::string &msg)
{
    msg.clear();

    if (incoming_size_ == 0)
    {
        return false;
    }

    std::size_t scanned = 0;
    while (scanned < incoming_size_)
    {
        const char current = incoming_buffer_[(incoming_head_ + scanned) % incoming_capacity];
        ++scanned;

        if (current == '\n')
        {
            for (std::size_t i = 0; i < scanned - 1; ++i)
            {
                msg.push_back(incoming_buffer_[(incoming_head_ + i) % incoming_capacity]);
            }

            if (!msg.empty() && msg.back() == '\r')
            {
                msg.pop_back();
            }

            incoming_head_ = (incoming_head_ + scanned) % incoming_capacity;
            incoming_size_ -= scanned;
            return true;
        }
    }

    return false;
}
