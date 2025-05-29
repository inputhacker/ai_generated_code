#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <chrono>
#include <sstream>

struct Message {
    enum class Role { USER, ASSISTANT };
    
    Role role;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
    
    Message(Role r, const std::string& c);
    
    std::string getRoleString() const;
    std::string toJSON() const;
};

#endif // MESSAGE_HPP
