#ifndef LLM_AGENT_HPP
#define LLM_AGENT_HPP

#include "Message.hpp"
#include <vector>
#include <string>
#include <memory>

// LLM Agent 인터페이스 클래스
class LLMAgent {
public:
    virtual ~LLMAgent() = default;
    virtual std::string generateResponse(const std::string& prompt) = 0;
    virtual std::vector<float> generateEmbedding(const std::string& text) = 0;
    virtual std::string summarizeConversation(const std::vector<Message>& messages) = 0;
};

// ChatGPT 구현
class ChatGPTAgent : public LLMAgent {
public:
    std::string generateResponse(const std::string& prompt) override;
    std::vector<float> generateEmbedding(const std::string& text) override;
    std::string summarizeConversation(const std::vector<Message>& messages) override;
};

// Claude 구현
class ClaudeAgent : public LLMAgent {
public:
    std::string generateResponse(const std::string& prompt) override;
    std::vector<float> generateEmbedding(const std::string& text) override;
    std::string summarizeConversation(const std::vector<Message>& messages) override;
};

#endif // LLM_AGENT_HPP
