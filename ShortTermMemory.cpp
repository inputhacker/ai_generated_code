#include "ShortTermMemory.hpp"
#include <sstream>

ShortTermMemory::ShortTermMemory(std::shared_ptr<LLMAgent> agent, 
                                size_t maxSize, size_t threshold, size_t keepRecent)
    : llmAgent(agent), 
      maxContextSize(maxSize), 
      summaryThreshold(threshold),
      recentMessagesToKeep(keepRecent) {}

void ShortTermMemory::addMessage(const Message& message) {
    conversationHistory.push_back(message);
    
    if (getCurrentContextSize() > summaryThreshold) {
        summarizeOlderMessages();
    }
}

size_t ShortTermMemory::getCurrentContextSize() const {
    size_t totalSize = 0;
    for (const auto& msg : conversationHistory) {
        totalSize += msg.content.size();
    }
    return totalSize;
}

void ShortTermMemory::summarizeOlderMessages() {
    if (conversationHistory.size() <= recentMessagesToKeep) {
        return;
    }
    
    std::vector<Message> messagesToSummarize(
        conversationHistory.begin(), 
        conversationHistory.end() - recentMessagesToKeep
    );
    
    std::vector<Message> messagesToKeep(
        conversationHistory.end() - recentMessagesToKeep, 
        conversationHistory.end()
    );
    
    std::string summary = llmAgent->summarizeConversation(messagesToSummarize);
    
    conversationHistory.clear();
    conversationHistory.push_back(Message(Message::Role::ASSISTANT, 
                                         "대화 요약: " + summary));
    
    for (const auto& msg : messagesToKeep) {
        conversationHistory.push_back(msg);
    }
}

std::string ShortTermMemory::getContextForNewQuery(const std::string& newQuery) const {
    std::stringstream contextStream;
    
    for (const auto& msg : conversationHistory) {
        contextStream << msg.getRoleString() << ": " << msg.content << "\n\n";
    }
    
    contextStream << "user: " << newQuery;
    
    return contextStream.str();
}

std::string ShortTermMemory::getConversationHistoryJSON() const {
    std::stringstream jsonStream;
    jsonStream << "[";
    
    for (size_t i = 0; i < conversationHistory.size(); ++i) {
        jsonStream << conversationHistory[i].toJSON();
        if (i < conversationHistory.size() - 1) {
            jsonStream << ",";
        }
    }
    
    jsonStream << "]";
    return jsonStream.str();
}

std::vector<Message> ShortTermMemory::getConversationHistory() const {
    return conversationHistory;
}

void ShortTermMemory::clearHistory() {
    conversationHistory.clear();
}
