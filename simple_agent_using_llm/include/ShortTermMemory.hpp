#ifndef SHORT_TERM_MEMORY_HPP
#define SHORT_TERM_MEMORY_HPP

#include "Message.hpp"
#include "LLMAgent.hpp"
#include <vector>
#include <string>
#include <memory>

class ShortTermMemory {
private:
    std::vector<Message> conversationHistory;
    size_t maxContextSize;
    size_t summaryThreshold;
    size_t recentMessagesToKeep;
    std::shared_ptr<LLMAgent> llmAgent;
    
public:
    ShortTermMemory(std::shared_ptr<LLMAgent> agent, 
                   size_t maxSize = 10000, 
                   size_t threshold = 8000, 
                   size_t keepRecent = 5);
    
    void addMessage(const Message& message);
    size_t getCurrentContextSize() const;
    void summarizeOlderMessages();
    std::string getContextForNewQuery(const std::string& newQuery) const;
    std::string getConversationHistoryJSON() const;
    std::vector<Message> getConversationHistory() const;
    void clearHistory();
};

#endif // SHORT_TERM_MEMORY_HPP
