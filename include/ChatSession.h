#ifndef CHAT_SESSION_H
#define CHAT_SESSION_H

#include <atomic>
#include <cstddef>
#include <array>
#include <memory>
#include <string>
#include <utility>
#include <asio.hpp>

// forward declaration
class ChatRoom;

using asio::ip::tcp;

template <typename T, std::size_t Capacity>
class AtomicRingBuffer
{
public:
    static_assert(Capacity > 0, "AtomicRingBuffer capacity must be greater than zero");

    AtomicRingBuffer() = default;
    AtomicRingBuffer(const AtomicRingBuffer &) = delete;
    AtomicRingBuffer &operator=(const AtomicRingBuffer &) = delete;

    std::size_t readable_size() const
    {
        const std::size_t head = head_.load(std::memory_order_acquire);
        const std::size_t tail = tail_.load(std::memory_order_acquire);

        if (tail >= head)
        {
            return tail - head;
        }

        return storage_capacity - (head - tail);
    }

    std::size_t writable_size() const
    {
        return Capacity - readable_size();
    }

    bool push(const T &value)
    {
        std::size_t writable_length = 0;
        T *write_ptr = writable_data(writable_length);

        if (write_ptr == nullptr || writable_length == 0)
        {
            return false;
        }

        *write_ptr = value;
        commit_write(1);
        return true;
    }

    bool push(T &&value)
    {
        std::size_t writable_length = 0;
        T *write_ptr = writable_data(writable_length);

        if (write_ptr == nullptr || writable_length == 0)
        {
            return false;
        }

        *write_ptr = std::move(value);
        commit_write(1);
        return true;
    }

    T *writable_data(std::size_t &length)
    {
        const std::size_t head = head_.load(std::memory_order_acquire);
        const std::size_t tail = tail_.load(std::memory_order_relaxed);

        if (next_index(tail) == head)
        {
            length = 0;
            return nullptr;
        }

        if (tail >= head)
        {
            length = storage_capacity - tail - (head == 0 ? 1 : 0);
        }
        else
        {
            length = head - tail - 1;
        }

        return buffer_.data() + tail;
    }

    const T *readable_data(std::size_t &length) const
    {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t tail = tail_.load(std::memory_order_acquire);

        if (head == tail)
        {
            length = 0;
            return nullptr;
        }

        if (head < tail)
        {
            length = tail - head;
        }
        else
        {
            length = storage_capacity - head;
        }

        return buffer_.data() + head;
    }

    void commit_write(std::size_t length)
    {
        if (length == 0)
        {
            return;
        }

        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        tail_.store(advance_index(tail, length), std::memory_order_release);
    }

    void consume(std::size_t length)
    {
        if (length == 0)
        {
            return;
        }

        const std::size_t head = head_.load(std::memory_order_relaxed);
        head_.store(advance_index(head, length), std::memory_order_release);
    }

    const T &at(std::size_t offset) const
    {
        const std::size_t head = head_.load(std::memory_order_acquire);
        return buffer_[advance_index(head, offset)];
    }

private:
    static constexpr std::size_t storage_capacity = Capacity + 1;

    static std::size_t next_index(std::size_t index)
    {
        return (index + 1) % storage_capacity;
    }

    static std::size_t advance_index(std::size_t index, std::size_t offset)
    {
        return (index + offset) % storage_capacity;
    }

    std::array<T, storage_capacity> buffer_{};
    std::atomic<std::size_t> head_{0};
    std::atomic<std::size_t> tail_{0};
};

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
    bool try_pop_incoming_message(std::string &msg);

    tcp::socket socket_;
    asio::strand<asio::any_io_executor> strand_;
    ChatRoom &room_;

    static constexpr std::size_t incoming_capacity = 4096;
    static constexpr std::size_t outgoing_capacity = 64;

    AtomicRingBuffer<char, incoming_capacity> incoming_buffer_;

    AtomicRingBuffer<std::string, outgoing_capacity> write_msgs_;
    bool writing_ = false;

    std::string username = "";
    bool init = false;
    bool joined_ = false;
};

#endif
