#include "ChatRoom.h"
#include "ChatSession.h"
#include <iostream>

void ChatRoom::join(std::shared_ptr<ChatSession> session)
{
          std::cout << "Client joined." << std::endl;
          sessions_.insert(session);
}

void ChatRoom::leave(std::shared_ptr<ChatSession> session)
{
          std::cout << "Client left." << std::endl;
          sessions_.erase(session);
}

void ChatRoom::broadcast(const std::string &msg, std::shared_ptr<ChatSession> sender)
{
          // iterate through all the sessions and send the message to each individually
          for (auto session : sessions_)
          {
                    if (session != sender)
                    {
                              session->deliver(msg);
                    }
          }
}
