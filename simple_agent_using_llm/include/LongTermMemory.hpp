#ifndef LONG_TERM_MEMORY_HPP
#define LONG_TERM_MEMORY_HPP

#include "Message.hpp"
#include "LLMAgent.hpp"
#include "VectorDB.hpp"
#include <vector>
#include <string>
#include <memory>
#include <mutex>

class LongTermMemory {
private:
    std::shared_ptr<VectorDB> vectorDB;
    std::shared_ptr<LLMAgent> llmAgent;
    std::mutex dbMutex;
    
public:
    LongTermMemory(std::shared_ptr<VectorDB> db, std::shared_ptr<LLMAgent> agent);
    
    void storeConversation(const std::string& sessionId, const std::vector<Message>& conversation);
    std::vector<std::string> retrieveSimilarConversations(const std::string& query, int topK = 5);
};

#endif // LONG_TERM_MEMORY_HPP
