#include "response.hpp"
#include <sstream>
#include <iomanip>

Response::Response() {
    timestamp = std::chrono::system_clock::now();
}

Response::Response(const std::string& text) : Response() {
    content.addText(text);
}

Response::Response(const Modality& content) : Response() {
    this->content = content;
}

void Response::setMetadata(const std::string& key, const std::string& value) {
    metadata[key] = value;
}

std::string Response::getMetadata(const std::string& key) const {
    auto it = metadata.find(key);
    if (it != metadata.end()) {
        return it->second;
    }
    return "";
}

bool Response::hasMetadata(const std::string& key) const {
    return metadata.find(key) != metadata.end();
}

std::string Response::serialize() const {
    std::stringstream ss;
    ss << "{";
    
    // 컨텐츠 직렬화
    ss << "\"content\":" << content.serialize();
    
    // 상태 직렬화
    ss << ",\"status\":" << static_cast<int>(status);
    
    // 오류 메시지 직렬화
    if (!errorMessage.empty()) {
        ss << ",\"errorMessage\":\"";
        for (char c : errorMessage) {
            if (c == '\"') ss << "\\\"";
            else if (c == '\\') ss << "\\\\";
            else ss << c;
        }
        ss << "\"";
    }
    
    // 메타데이터 직렬화
    if (!metadata.empty()) {
        ss << ",\"metadata\":{";
        bool first = true;
        for (const auto& [key, value] : metadata) {
            if (!first) ss << ",";
            ss << "\"" << key << "\":\"" << value << "\"";
            first = false;
        }
        ss << "}";
    }
    
    // 타임스탬프 직렬화
    auto timeT = std::chrono::system_clock::to_time_t(timestamp);
    ss << ",\"timestamp\":\"" << std::put_time(std::gmtime(&timeT), "%FT%TZ") << "\"";
    
    ss << "}";
    return ss.str();
}

Response Response::deserialize(const std::string& data) {
    // 주의: 이 구현은 간단한 예시입니다. 실제로는 JSON 라이브러리 사용을 권장합니다.
    Response response;
    
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
        response.setContent(Modality::deserialize(contentJson));
    }
    
    // status 파싱
    size_t statusStart = data.find("\"status\":");
    if (statusStart != std::string::npos) {
        statusStart += 9;  // "status": 길이
        size_t statusEnd = data.find_first_of(",}", statusStart);
        if (statusEnd != std::string::npos) {
            std::string statusStr = data.substr(statusStart, statusEnd - statusStart);
            response.setStatus(static_cast<ResponseStatus>(std::stoi(statusStr)));
        }
    }
    
    // errorMessage 파싱
    size_t errorStart = data.find("\"errorMessage\":\"");
    if (errorStart != std::string::npos) {
        errorStart += 15;  // "errorMessage":"의 길이
        size_t errorEnd = data.find("\"", errorStart);
        while (errorEnd != std::string::npos && data[errorEnd-1] == '\\') {
            errorEnd = data.find("\"", errorEnd + 1);
        }
        if (errorEnd != std::string::npos) {
            std::string errorMsg = data.substr(errorStart, errorEnd - errorStart);
            // 이스케이프된 문자 처리
            std::string unescaped;
            for (size_t i = 0; i < errorMsg.length(); ++i) {
                if (errorMsg[i] == '\\' && i + 1 < errorMsg.length()) {
                    switch (errorMsg[i + 1]) {
                        case '\"': unescaped.push_back('\"'); ++i; break;
                        case '\\': unescaped.push_back('\\'); ++i; break;
                        default: unescaped.push_back(errorMsg[i]);
                    }
                } else {
                    unescaped.push_back(errorMsg[i]);
                }
            }
            response.setErrorMessage(unescaped);
        }
    }
    
    // metadata 파싱
    size_t metaStart = data.find("\"metadata\":{");
    if (metaStart != std::string::npos) {
        metaStart += 12;  // "metadata":{ 길이
        
        // metadata 객체의 끝 찾기
        int bracketCount = 1;
        size_t metaEnd = metaStart;
        bool inQuotes = false;
        
        for (; metaEnd < data.length(); ++metaEnd) {
            char c = data[metaEnd];
            
            if (c == '\"' && (metaEnd == 0 || data[metaEnd-1] != '\\')) {
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
        
        // 간단한 메타데이터 파싱
        std::string metaJson = data.substr(metaStart, metaEnd - metaStart);
        size_t pos = 0;
        while ((pos = metaJson.find("\"", pos)) != std::string::npos) {
            size_t keyStart = pos + 1;
            size_t keyEnd = metaJson.find("\"", keyStart);
            if (keyEnd == std::string::npos) break;
            
            std::string key = metaJson.substr(keyStart, keyEnd - keyStart);
            
            size_t valueStart = metaJson.find("\"", keyEnd + 1);
            if (valueStart == std::string::npos) break;
            valueStart++;
            
            size_t valueEnd = metaJson.find("\"", valueStart);
            if (valueEnd == std::string::npos) break;
            
            std::string value = metaJson.substr(valueStart, valueEnd - valueStart);
            
            response.setMetadata(key, value);
            pos = valueEnd + 1;
        }
    }
    
    // timestamp는 파싱하지 않고 현재 시간 사용 (실제로는 파싱 필요)
    // 이 간단한 구현에서는 타임스탬프를 현재 시간으로 설정
    response.timestamp = std::chrono::system_clock::now();
    
    return response;
}
