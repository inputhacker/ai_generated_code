#ifndef QUERY_HPP
#define QUERY_HPP

#include "modality.hpp"
#include <map>
#include <string>

/**
 * @brief LLM에 보낼 쿼리를 나타내는 클래스
 */
class Query {
public:
    Query() = default;
    explicit Query(const std::string& text);
    explicit Query(const Modality& content);

    // 컨텐츠 관련 메서드
    Modality& getContent() { return content; }
    const Modality& getContent() const { return content; }
    void setContent(const Modality& content) { this->content = content; }

    // 매개변수 관련 메서드
    void setParameter(const std::string& key, const std::string& value);
    std::string getParameter(const std::string& key) const;
    bool hasParameter(const std::string& key) const;
    const std::map<std::string, std::string>& getParameters() const { return parameters; }

    // 세션 정보
    void setUserId(const std::string& id) { userId = id; }
    std::string getUserId() const { return userId; }
    
    void setSessionId(const std::string& id) { sessionId = id; }
    std::string getSessionId() const { return sessionId; }

    // 직렬화/역직렬화
    std::string serialize() const;
    static Query deserialize(const std::string& data);

private:
    Modality content;
    std::map<std::string, std::string> parameters;
    std::string userId;
    std::string sessionId;
};

#endif // QUERY_HPP
