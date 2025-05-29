#include "UserAgent.hpp"
#include <iostream>

int main() {
    // UserAgent 생성
    UserAgent agent;
    
    std::cout << "LLM Agent 대화 시스템이 시작되었습니다. 종료하려면 'exit'를 입력하세요." << std::endl;
    
    std::string query;
    while (true) {
        std::cout << "\n사용자: ";
        std::getline(std::cin, query);
        
        if (query == "exit") {
            break;
        }
        else if (query.substr(0, 6) == "switch") {
            std::string agentName = query.substr(7);
            if (agent.switchAgent(agentName)) {
                std::cout << "에이전트가 " << agentName << "로 전환되었습니다." << std::endl;
            } else {
                std::cout << "사용 가능한 에이전트가 아닙니다." << std::endl;
            }
            continue;
        }
        else if (query == "new session") {
            agent.saveSession();
            agent.startNewSession();
            std::cout << "새 세션이 시작되었습니다." << std::endl;
            continue;
        }
        
        std::string response = agent.processQuery(query);
        std::cout << "\n어시스턴트: " << response << std::endl;
    }
    
    // 세션 저장 후 종료
    agent.saveSession();
    std::cout << "시스템을 종료합니다." << std::endl;
    
    return 0;
}
