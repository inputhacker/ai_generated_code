#ifndef LLM_CLIENT_HPP
#define LLM_CLIENT_HPP

#include "query.hpp"
#include "response.hpp"
#include <string>
#include <functional>
#include <future>

/**
 * @brief LLM API와 통신하는 클라이언트 클래스
 */
class LLMClient {
public:
    using ResponseCallback = std::function<void(const Response&)>;
    
    LLMClient() = default;
    LLMClient(const std::string& apiKey, const std::string& endpoint);
    ~LLMClient();

    // API 설정
    void setApiKey(const std::string& key) { apiKey = key; }
    std::string getApiKey() const { return apiKey; }
    
    void setApiEndpoint(const std::string& endpoint) { apiEndpoint = endpoint; }
    std::string getApiEndpoint() const { return apiEndpoint; }
    
    // 스트리밍 설정
    void setStreaming(bool enabled) { isStreaming = enabled; }
    bool getStreaming() const { return isStreaming; }
    
    // 타임아웃 설정
    void setTimeout(unsigned int milliseconds) { timeoutMs = milliseconds; }
    unsigned int getTimeout() const { return timeoutMs; }
    
    // 쿼리 전송 메서드
    Response sendQuery(const Query& query);                            // 동기 호출
    std::future<Response> sendQueryAsync(const Query& query);          // 비동기 호출
    bool streamQuery(const Query& query, ResponseCallback callback);   // 스트리밍 호출
    
    // 진행 중인 요청 취소
    bool cancelRequest();

private:
    std::string apiKey;
    std::string apiEndpoint;
    bool isStreaming = false;
    unsigned int timeoutMs = 30000;  // 기본값: 30초
    bool requestInProgress = false;
    bool cancelRequested = false;
};

#endif // LLM_CLIENT_HPP
