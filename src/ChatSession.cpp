#include "ChatSession.h"
#include "ChatRoom.h"
#include <iostream>

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
                   if (!self->write_msgs_.writable_size() || !self->write_msgs_.push(msg))
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
    if (incoming_buffer_.writable_size() == 0)
    {
        if (joined_)
        {
            room_.leave(shared_from_this());
            joined_ = false;
        }
        return;
    }

    std::size_t contiguous_space = 0;
    char *write_buffer = incoming_buffer_.writable_data(contiguous_space);

    if (write_buffer == nullptr || contiguous_space == 0)
    {
        if (joined_)
        {
            room_.leave(shared_from_this());
            joined_ = false;
        }

        return;
    }

    socket_.async_read_some(
        asio::buffer(write_buffer, contiguous_space),
        asio::bind_executor(
            strand_,
            [this, self = shared_from_this()](std::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    incoming_buffer_.commit_write(length);
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
    if (write_msgs_.readable_size() == 0)
    {
        writing_ = false;
        return;
    }

    std::size_t readable_length = 0;
    const std::string *message = write_msgs_.readable_data(readable_length);

    if (message == nullptr || readable_length == 0)
    {
        writing_ = false;
        return;
    }

    writing_ = true;

    asio::async_write(socket_,
                      asio::buffer(*message),
                      asio::bind_executor(
                          strand_,
                          [this, self = shared_from_this()](std::error_code ec, std::size_t)
                          {
                              if (!ec)
                              {
                                  write_msgs_.consume(1);

                                  if (write_msgs_.readable_size() > 0)
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

bool ChatSession::try_pop_incoming_message(std::string &msg)
{
    msg.clear();

    const std::size_t available = incoming_buffer_.readable_size();

    if (available == 0)
    {
        return false;
    }

    for (std::size_t scanned = 0; scanned < available; ++scanned)
    {
        const char current = incoming_buffer_.at(scanned);

        if (current == '\n')
        {
            for (std::size_t i = 0; i < scanned; ++i)
            {
                msg.push_back(incoming_buffer_.at(i));
            }

            if (!msg.empty() && msg.back() == '\r')
            {
                msg.pop_back();
            }

            incoming_buffer_.consume(scanned + 1);
            return true;
        }
    }

    return false;
}
