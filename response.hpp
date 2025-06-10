#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "modality.hpp"
#include <map>
#include <string>
#include <chrono>

/**
 * @brief 응답 상태 열거형
 */
enum class ResponseStatus {
    SUCCESS,    // 성공적인 응답
    PARTIAL,    // 부분 응답 (스트리밍에서 사용)
    ERROR,      // 오류 발생
    THROTTLED,  // 요청이 제한됨
    CANCELLED   // 사용자에 의해 취소됨
};

/**
 * @brief LLM에서 받은 응답을 나타내는 클래스
 */
class Response {
public:
    Response();
    explicit Response(const std::string& text);
    explicit Response(const Modality& content);

    // 컨텐츠 관련 메서드
    Modality& getContent() { return content; }
    const Modality& getContent() const { return content; }
    void setContent(const Modality& content) { this->content = content; }

    // 상태 관련 메서드
    ResponseStatus getStatus() const { return status; }
    void setStatus(ResponseStatus status) { this->status = status; }
    
    std::string getErrorMessage() const { return errorMessage; }
    void setErrorMessage(const std::string& message) { errorMessage = message; }

    // 메타데이터 관련 메서드
    void setMetadata(const std::string& key, const std::string& value);
    std::string getMetadata(const std::string& key) const;
    bool hasMetadata(const std::string& key) const;
    const std::map<std::string, std::string>& getMetadata() const { return metadata; }

    // 타임스탬프 관련 메서드
    std::chrono::system_clock::time_point getTimestamp() const { return timestamp; }

    // 직렬화/역직렬화
    std::string serialize() const;
    static Response deserialize(const std::string& data);

private:
    Modality content;
    ResponseStatus status = ResponseStatus::SUCCESS;
    std::string errorMessage;
    std::map<std::string, std::string> metadata;
    std::chrono::system_clock::time_point timestamp;
};

#endif // RESPONSE_HPP
