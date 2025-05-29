#include "Message.hpp"

Message::Message(Role r, const std::string& c) 
    : role(r), content(c), timestamp(std::chrono::system_clock::now()) {}

std::string Message::getRoleString() const {
    return role == Role::USER ? "user" : "assistant";
}

std::string Message::toJSON() const {
    std::stringstream ss;
    ss << "{\"role\":\"" << getRoleString() << "\",\"content\":\"" << content << "\"}";
    return ss.str();
}
