#include "UserAgent.hpp"
#include <iostream>

UserAgent::UserAgent() {
    // 벡터 DB 및 LLM Agent 인스턴스 생성
    auto vectorDB = std::make_shared<SimpleVectorDB>();
    
    // LLM 에이전트 등록
    llmAgents["chatgpt"] = std::make_shared<ChatGPTAgent>();
    llmAgents["claude"] = std::make_shared<ClaudeAgent>();
    
    // 기본 에이전트 설정
    currentAgentId = "chatgpt";
    
    // Memory Manager 초기화
    memoryManager = std::make_shared<MemoryManager>(
        llmAgents[currentAgentId], vectorDB);
}

bool UserAgent::switchAgent(const std::string& agentId) {
    if (llmAgents.find(agentId) != llmAgents.end()) {
        currentAgentId = agentId;
        return true;
    }
    return false;
}

std::string UserAgent::processQuery(const std::string& query) {
    // 사용자 메시지 저장
    memoryManager->addUserMessage(query);
    
    // 관련 컨텍스트 준비
    std::string prompt = memoryManager->preparePromptForNewQuery(query);

    std::cout << "[프롬프트--시작]" << std::endl << prompt << std::endl << "[프롬프트--끝]" << std::endl;
    
    // 장기 기억에서 관련 대화 검색 (필요시)
    std::vector<std::string> relevantConversations;
    if (query.length() > 0) { // 간단한 예시 조건
        relevantConversations = memoryManager->retrieveRelevantConversations(query);
        
        // 관련 대화가 있으면 프롬프트에 추가
        if (!relevantConversations.empty()) {
            prompt = "다음은 이전에 유사한 주제로 나눈 대화입니다:\n\n" + 
                     relevantConversations[0] + "\n\n현재 대화:\n\n" + prompt;
        }
    }
    
    // 현재 에이전트로 응답 생성
    std::string response = llmAgents[currentAgentId]->generateResponse(prompt);
    
    // 응답 저장
    memoryManager->addAssistantResponse(response);
    
    return response;
}

void UserAgent::startNewSession() {
    memoryManager->startNewSession();
}

void UserAgent::saveSession() {
    memoryManager->saveCurrentSession();
}
