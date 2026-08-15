#include "ChatRoom.h"
#include "ChatSession.h"
#include <iostream>
#include <mutex>
#include <vector>

void ChatRoom::join(std::shared_ptr<ChatSession> session)
{
    std::cout << "Client joined." << std::endl;
    std::unique_lock lock(sessions_mutex_);
    sessions_.insert(session);
}

void ChatRoom::leave(std::shared_ptr<ChatSession> session)
{
    std::cout << "Client left." << std::endl;
    std::unique_lock lock(sessions_mutex_);
    sessions_.erase(session);
}

void ChatRoom::broadcast(const std::string &msg, std::shared_ptr<ChatSession> sender)
{
    std::vector<std::shared_ptr<ChatSession>> recipients;

    {
        std::shared_lock lock(sessions_mutex_);
        recipients.reserve(sessions_.size());

        for (const auto &session : sessions_)
        {
            if (session != sender)
            {
                recipients.push_back(session);
            }
        }
    }

    // iterate through the snapshot and send the message to each individually
    for (const auto &session : recipients)
    {
        session->deliver(msg);
    }
}
