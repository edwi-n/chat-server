#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include <set>
#include <memory>
#include <string>

// forward declaration so that ChatSession->deliver can get called
class ChatSession;

// stores all active sessions and can be used to broadcast messages to each session
class ChatRoom
{
public:
          void join(std::shared_ptr<ChatSession> session);
          void leave(std::shared_ptr<ChatSession> session);
          void broadcast(const std::string &msg, std::shared_ptr<ChatSession> sender);

private:
          std::set<std::shared_ptr<ChatSession>> sessions_; // set of all the sessions
};

#endif
