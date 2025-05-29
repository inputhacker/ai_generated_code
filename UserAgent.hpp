#ifndef USER_AGENT_HPP
#define USER_AGENT_HPP

#include "MemoryManager.hpp"
#include "LLMAgent.hpp"
#include <memory>
#include <string>
#include <unordered_map>

class UserAgent {
private:
    std::shared_ptr<MemoryManager> memoryManager;
    std::unordered_map<std::string, std::shared_ptr<LLMAgent>> llmAgents;
    std::string currentAgentId;
    
public:
    UserAgent();
    
    bool switchAgent(const std::string& agentId);
    std::string processQuery(const std::string& query);
    void startNewSession();
    void saveSession();
};

#endif // USER_AGENT_HPP
