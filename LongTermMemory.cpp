#include "LongTermMemory.hpp"

LongTermMemory::LongTermMemory(std::shared_ptr<VectorDB> db, std::shared_ptr<LLMAgent> agent)
    : vectorDB(db), llmAgent(agent) {}

void LongTermMemory::storeConversation(const std::string& sessionId, const std::vector<Message>& conversation) {
    std::string conversationText;
    for (const auto& msg : conversation) {
        conversationText += msg.getRoleString() + ": " + msg.content + "\n";
    }
    
    std::vector<float> embedding = llmAgent->generateEmbedding(conversationText);
    
    std::stringstream metadata;
    metadata << "{\"session_id\":\"" << sessionId 
             << "\",\"timestamp\":\"" << std::chrono::system_clock::now().time_since_epoch().count()
             << "\",\"conversation\":\"" << conversationText << "\"}";
    
    std::lock_guard<std::mutex> lock(dbMutex);
    vectorDB->store(sessionId, embedding, metadata.str());
}

std::vector<std::string> LongTermMemory::retrieveSimilarConversations(const std::string& query, int topK) {
    std::vector<float> queryEmbedding = llmAgent->generateEmbedding(query);
    
    std::lock_guard<std::mutex> lock(dbMutex);
    auto results = vectorDB->search(queryEmbedding, topK);
    
    std::vector<std::string> conversations;
    for (const auto& result : results) {
        conversations.push_back(result.first);
    }
    
    return conversations;
}
