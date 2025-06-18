#include "query.hpp"
#include <sstream>

Query::Query(const std::string& text) {
    content.addText(text);
}

Query::Query(const Modality& content) : content(content) {}

void Query::setParameter(const std::string& key, const std::string& value) {
    parameters[key] = value;
}

std::string Query::getParameter(const std::string& key) const {
    auto it = parameters.find(key);
    if (it != parameters.end()) {
        return it->second;
    }
    return "";
}

bool Query::hasParameter(const std::string& key) const {
    return parameters.find(key) != parameters.end();
}

std::string Query::serialize() const {
    std::stringstream ss;
    ss << "{";
    
    // 컨텐츠 직렬화
    ss << "\"content\":" << content.serialize();
    
    // 매개변수 직렬화
    if (!parameters.empty()) {
        ss << ",\"parameters\":{";
        bool first = true;
        for (const auto& [key, value] : parameters) {
            if (!first) ss << ",";
            ss << "\"" << key << "\":\"" << value << "\"";
            first = false;
        }
        ss << "}";
    }
    
    // 세션 정보 직렬화
    if (!userId.empty()) {
        ss << ",\"userId\":\"" << userId << "\"";
    }
    
    if (!sessionId.empty()) {
        ss << ",\"sessionId\":\"" << sessionId << "\"";
    }
    
    ss << "}";
    return ss.str();
}

Query Query::deserialize(const std::string& data) {
    // 주의: 이 구현은 간단한 예시입니다. 실제로는 JSON 라이브러리 사용을 권장합니다.
    Query query;
    
    // content 파싱
    size_t contentStart = data.find("\"content\":");
    if (contentStart != std::string::npos) {
        contentStart += 10;  // "content": 길이
        
        // content 객체의 끝 찾기
        int bracketCount = 0;
        size_t contentEnd = contentStart;
        bool inQuotes = false;
        
        for (; contentEnd < data.length(); ++contentEnd) {
            char c = data[contentEnd];
            
            if (c == '\"' && (contentEnd == 0 || data[contentEnd-1] != '\\')) {
                inQuotes = !inQuotes;
            }
            
            if (!inQuotes) {
                if (c == '{') bracketCount++;
                else if (c == '}') {
                    bracketCount--;
                    if (bracketCount == 0) {
                        contentEnd++;
                        break;
                    }
                }
            }
        }
        
        std::string contentJson = data.substr(contentStart, contentEnd - contentStart);
        query.setContent(Modality::deserialize(contentJson));
    }
    
    // parameters 파싱
    size_t paramsStart = data.find("\"parameters\":{");
    if (paramsStart != std::string::npos) {
        paramsStart += 14;  // "parameters":{ 길이
        
        // parameters 객체의 끝 찾기
        int bracketCount = 1;
        size_t paramsEnd = paramsStart;
        bool inQuotes = false;
        
        for (; paramsEnd < data.length(); ++paramsEnd) {
            char c = data[paramsEnd];
            
            if (c == '\"' && (paramsEnd == 0 || data[paramsEnd-1] != '\\')) {
                inQuotes = !inQuotes;
            }
            
            if (!inQuotes) {
                if (c == '{') bracketCount++;
                else if (c == '}') {
                    bracketCount--;
                    if (bracketCount == 0) break;
                }
            }
        }
        
        // 간단한 파라미터 파싱 (더 복잡한 구현이 필요할 수 있음)
        std::string paramsJson = data.substr(paramsStart, paramsEnd - paramsStart);
        size_t pos = 0;
        while ((pos = paramsJson.find("\"", pos)) != std::string::npos) {
            size_t keyStart = pos + 1;
            size_t keyEnd = paramsJson.find("\"", keyStart);
            if (keyEnd == std::string::npos) break;
            
            std::string key = paramsJson.substr(keyStart, keyEnd - keyStart);
            
            size_t valueStart = paramsJson.find("\"", keyEnd + 1);
            if (valueStart == std::string::npos) break;
            valueStart++;
            
            size_t valueEnd = paramsJson.find("\"", valueStart);
            if (valueEnd == std::string::npos) break;
            
            std::string value = paramsJson.substr(valueStart, valueEnd - valueStart);
            
            query.setParameter(key, value);
            pos = valueEnd + 1;
        }
    }
    
    // userId 파싱
    size_t userIdStart = data.find("\"userId\":\"");
    if (userIdStart != std::string::npos) {
        userIdStart += 10;  // "userId":"의 길이
        size_t userIdEnd = data.find("\"", userIdStart);
        if (userIdEnd != std::string::npos) {
            query.setUserId(data.substr(userIdStart, userIdEnd - userIdStart));
        }
    }
    
    // sessionId 파싱
    size_t sessionIdStart = data.find("\"sessionId\":\"");
    if (sessionIdStart != std::string::npos) {
        sessionIdStart += 13;  // "sessionId":"의 길이
        size_t sessionIdEnd = data.find("\"", sessionIdStart);
        if (sessionIdEnd != std::string::npos) {
            query.setSessionId(data.substr(sessionIdStart, sessionIdEnd - sessionIdStart));
        }
    }
    
    return query;
}
