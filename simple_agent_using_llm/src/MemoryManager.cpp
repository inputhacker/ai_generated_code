#include "MemoryManager.hpp"

MemoryManager::MemoryManager(std::shared_ptr<LLMAgent> agent, std::shared_ptr<VectorDB> db) {
    shortTermMemory = std::make_shared<ShortTermMemory>(agent);
    longTermMemory = std::make_shared<LongTermMemory>(db, agent);
    currentSessionId = generateSessionId();
}

std::string MemoryManager::generateSessionId() {
    auto now = std::chrono::system_clock::now();
    return "session_" + std::to_string(now.time_since_epoch().count());
}

void MemoryManager::startNewSession() {
    saveCurrentSession();
    currentSessionId = generateSessionId();
    shortTermMemory->clearHistory();
}

void MemoryManager::saveCurrentSession() {
    auto conversation = shortTermMemory->getConversationHistory();
    if (!conversation.empty()) {
        longTermMemory->storeConversation(currentSessionId, conversation);
    }
}

void MemoryManager::addUserMessage(const std::string& message) {
    shortTermMemory->addMessage(Message(Message::Role::USER, message));
}

void MemoryManager::addAssistantResponse(const std::string& response) {
    shortTermMemory->addMessage(Message(Message::Role::ASSISTANT, response));
}

std::string MemoryManager::preparePromptForNewQuery(const std::string& newQuery) {
    return shortTermMemory->getContextForNewQuery(newQuery);
}

std::vector<std::string> MemoryManager::retrieveRelevantConversations(const std::string& query, int topK) {
    return longTermMemory->retrieveSimilarConversations(query, topK);
}

std::shared_ptr<ShortTermMemory> MemoryManager::getShortTermMemory() {
    return shortTermMemory;
}

std::shared_ptr<LongTermMemory> MemoryManager::getLongTermMemory() {
    return longTermMemory;
}
