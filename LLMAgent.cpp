#include "LLMAgent.hpp"
#include <sstream>

// ChatGPT 구현
std::string ChatGPTAgent::generateResponse(const std::string& prompt) {
    // 실제 OpenAI API 호출 구현
    return "ChatGPT 응답 (실제 구현 필요)";
}

std::vector<float> ChatGPTAgent::generateEmbedding(const std::string& text) {
    // 실제 OpenAI Embedding API 호출 구현
    return std::vector<float>{0.1f, 0.2f}; // 예시
}

std::string ChatGPTAgent::summarizeConversation(const std::vector<Message>& messages) {
    std::stringstream prompt;
    prompt << "다음 대화를 간결하게 요약해주세요:\n\n";
    
    for (const auto& msg : messages) {
        prompt << msg.getRoleString() << ": " << msg.content << "\n";
    }
    
    return generateResponse(prompt.str());
}

// Claude 구현
std::string ClaudeAgent::generateResponse(const std::string& prompt) {
    // 실제 Claude API 호출 구현
    return "Claude 응답 (실제 구현 필요)";
}

std::vector<float> ClaudeAgent::generateEmbedding(const std::string& text) {
    // 실제 Claude Embedding API 호출 구현
    return std::vector<float>{0.3f, 0.4f}; // 예시
}

std::string ClaudeAgent::summarizeConversation(const std::vector<Message>& messages) {
    std::stringstream prompt;
    prompt << "Please summarize the following conversation concisely:\n\n";
    
    for (const auto& msg : messages) {
        prompt << msg.getRoleString() << ": " << msg.content << "\n";
    }
    
    return generateResponse(prompt.str());
}
