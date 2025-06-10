#include "modality.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <algorithm>

// 텍스트 관련 메서드 구현
void Modality::addText(const std::string& text) {
    this->text = text;
}

void Modality::clearText() {
    text.reset();
}

bool Modality::hasText() const {
    return text.has_value();
}

std::string Modality::getText() const {
    if (!hasText()) {
        throw std::runtime_error("No text content available");
    }
    return text.value();
}

// 이미지 관련 메서드 구현
void Modality::addImagePath(const std::string& path) {
    imagePath = path;
    imageData.reset();  // 이미지 경로가 설정되면 이미지 데이터는 초기화
}

void Modality::addImageData(const std::vector<uint8_t>& data, const std::string& mimeType) {
    imageData = data;
    imagePath.reset();  // 이미지 데이터가 설정되면 이미지 경로는 초기화
    imageMimeType = mimeType;
}

void Modality::clearImage() {
    imagePath.reset();
    imageData.reset();
}

bool Modality::hasImage() const {
    return hasImagePath() || hasImageData();
}

bool Modality::hasImagePath() const {
    return imagePath.has_value();
}

bool Modality::hasImageData() const {
    return imageData.has_value();
}

std::string Modality::getImagePath() const {
    if (!hasImagePath()) {
        throw std::runtime_error("No image path available");
    }
    return imagePath.value();
}

const std::vector<uint8_t>& Modality::getImageData() const {
    if (!hasImageData()) {
        throw std::runtime_error("No image data available");
    }
    return imageData.value();
}

std::string Modality::getImageMimeType() const {
    if (!hasImageData()) {
        throw std::runtime_error("No image data available");
    }
    return imageMimeType;
}

// Base64 인코딩 유틸리티 함수
static std::string base64_encode(const std::vector<uint8_t>& data) {
    static const char base64_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string result;
    int val = 0, valb = -6;
    
    for (uint8_t c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    while (result.size() % 4) {
        result.push_back('=');
    }
    
    return result;
}

// Base64 디코딩 유틸리티 함수
static std::vector<uint8_t> base64_decode(const std::string& encoded_string) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::vector<uint8_t> result;
    int val = 0, valb = -8;
    
    for (char c : encoded_string) {
        if (c == '=') {
            break;
        }
        
        size_t pos = base64_chars.find(c);
        if (pos == std::string::npos) {
            continue; // 유효하지 않은 문자는 무시
        }
        
        val = (val << 6) + static_cast<int>(pos);
        valb += 6;
        
        if (valb >= 0) {
            result.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    
    return result;
}

// 직렬화 메서드
std::string Modality::serialize() const {
    std::stringstream ss;
    ss << "{";
    
    // 텍스트 직렬화
    if (hasText()) {
        ss << "\"text\":\"";
        // 이스케이프 처리
        for (char c : text.value()) {
            switch (c) {
                case '\"': ss << "\\\""; break;
                case '\\': ss << "\\\\"; break;
                case '\b': ss << "\\b"; break;
                case '\f': ss << "\\f"; break;
                case '\n': ss << "\\n"; break;
                case '\r': ss << "\\r"; break;
                case '\t': ss << "\\t"; break;
                default:
                    if ('\x00' <= c && c <= '\x1f') {
                        ss << "\\u"
                           << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                    } else {
                        ss << c;
                    }
            }
        }
        ss << "\"";
    }
    
    // 이미지 경로 직렬화
    if (hasImagePath()) {
        if (hasText()) ss << ",";
        ss << "\"imagePath\":\"" << imagePath.value() << "\"";
    }
    
    // 이미지 데이터 직렬화
    if (hasImageData()) {
        if (hasText() || hasImagePath()) ss << ",";
        ss << "\"imageData\":\"" << base64_encode(imageData.value()) << "\","
           << "\"imageMimeType\":\"" << imageMimeType << "\"";
    }
    
    ss << "}";
    return ss.str();
}

// 역직렬화 메서드 (간단한 구현, 실제로는 JSON 라이브러리 사용 권장)
Modality Modality::deserialize(const std::string& data) {
    Modality result;
    
    // 간단한 파싱 (실제 구현에서는 JSON 라이브러리를 사용하는 것이 좋습니다)
    size_t textStart = data.find("\"text\":\"");
    if (textStart != std::string::npos) {
        textStart += 8; // "text":"의 길이
        size_t textEnd = data.find("\"", textStart);
        if (textEnd != std::string::npos) {
            std::string textValue = data.substr(textStart, textEnd - textStart);
            // 기본적인 이스케이프 시퀀스 처리
            std::string unescaped;
            for (size_t i = 0; i < textValue.length(); ++i) {
                if (textValue[i] == '\\' && i + 1 < textValue.length()) {
                    switch (textValue[i + 1]) {
                        case '"': unescaped.push_back('"'); ++i; break;
                        case '\\': unescaped.push_back('\\'); ++i; break;
                        case 'b': unescaped.push_back('\b'); ++i; break;
                        case 'f': unescaped.push_back('\f'); ++i; break;
                        case 'n': unescaped.push_back('\n'); ++i; break;
                        case 'r': unescaped.push_back('\r'); ++i; break;
                        case 't': unescaped.push_back('\t'); ++i; break;
                        default: unescaped.push_back(textValue[i]);
                    }
                } else {
                    unescaped.push_back(textValue[i]);
                }
            }
            result.addText(unescaped);
        }
    }
    
    size_t imagePathStart = data.find("\"imagePath\":\"");
    if (imagePathStart != std::string::npos) {
        imagePathStart += 13; // "imagePath":"의 길이
        size_t imagePathEnd = data.find("\"", imagePathStart);
        if (imagePathEnd != std::string::npos) {
            std::string imagePath = data.substr(imagePathStart, imagePathEnd - imagePathStart);
            result.addImagePath(imagePath);
        }
    }
    
    size_t imageDataStart = data.find("\"imageData\":\"");
    if (imageDataStart != std::string::npos) {
        imageDataStart += 13; // "imageData":"의 길이
        size_t imageDataEnd = data.find("\"", imageDataStart);
        if (imageDataEnd != std::string::npos) {
            std::string encodedData = data.substr(imageDataStart, imageDataEnd - imageDataStart);
            std::vector<uint8_t> decodedData = base64_decode(encodedData);
            
            // 이미지 MIME 타입 파싱
            std::string mimeType = "image/jpeg"; // 기본값
            size_t mimeTypeStart = data.find("\"imageMimeType\":\"");
            if (mimeTypeStart != std::string::npos) {
                mimeTypeStart += 17; // "imageMimeType":"의 길이
                size_t mimeTypeEnd = data.find("\"", mimeTypeStart);
                if (mimeTypeEnd != std::string::npos) {
                    mimeType = data.substr(mimeTypeStart, mimeTypeEnd - mimeTypeStart);
                }
            }
            
            result.addImageData(decodedData, mimeType);
        }
    }
    
    return result;
}
