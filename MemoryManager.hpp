#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include "ShortTermMemory.hpp"
#include "LongTermMemory.hpp"
#include <memory>
#include <string>
#include <vector>

class MemoryManager {
private:
    std::shared_ptr<ShortTermMemory> shortTermMemory;
    std::shared_ptr<LongTermMemory> longTermMemory;
    std::string currentSessionId;
    
public:
    MemoryManager(std::shared_ptr<LLMAgent> agent, std::shared_ptr<VectorDB> db);
    
    std::string generateSessionId();
    void startNewSession();
    void saveCurrentSession();
    void addUserMessage(const std::string& message);
    void addAssistantResponse(const std::string& response);
    std::string preparePromptForNewQuery(const std::string& newQuery);
    std::vector<std::string> retrieveRelevantConversations(const std::string& query, int topK = 3);
    
    std::shared_ptr<ShortTermMemory> getShortTermMemory();
    std::shared_ptr<LongTermMemory> getLongTermMemory();
};

#endif // MEMORY_MANAGER_HPP
