#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 대화 메시지 구조체
struct Message {
    std::string role;
    std::string text;
    std::string mimeType;  // 이미지인 경우 mime type (예: "image/jpeg")
    std::string imageData; // base64 encoded image data (있는 경우)
};

// Gemini API 요청 본문 생성 함수
json createRequestBody(
    const std::string& systemPrompt,
    const std::vector<Message>& conversationHistory,
    const Message& currentQuery,
    bool enableImageGeneration = false
) {
    json requestBody;
    json contents = json::array();
    
    // 1. System prompt 추가
    json systemMessage;
    systemMessage["role"] = "system";
    systemMessage["parts"] = json::array();
    systemMessage["parts"].push_back({{"text", systemPrompt}});
    contents.push_back(systemMessage);
    
    // 2. 이전 대화 내역 추가
    for (const auto& message : conversationHistory) {
        json messageJson;
        messageJson["role"] = message.role;
        messageJson["parts"] = json::array();
        
        if (!message.imageData.empty()) {
            // 이미지가 포함된 메시지
            messageJson["parts"].push_back({{"text", message.text}});
            messageJson["parts"].push_back({
                {"inline_data", {
                    {"mime_type", message.mimeType},
                    {"data", message.imageData}
                }}
            });
        } else {
            // 텍스트만 있는 메시지
            messageJson["parts"].push_back({{"text", message.text}});
        }
        
        contents.push_back(messageJson);
    }
    
    // 3. 현재 쿼리 추가
    json currentMessage;
    currentMessage["role"] = currentQuery.role;
    currentMessage["parts"] = json::array();
    
    if (!currentQuery.imageData.empty()) {
        // 이미지가 포함된 쿼리
        if (!currentQuery.text.empty()) {
            currentMessage["parts"].push_back({{"text", currentQuery.text}});
        }
        currentMessage["parts"].push_back({
            {"inline_data", {
                {"mime_type", currentQuery.mimeType},
                {"data", currentQuery.imageData}
            }}
        });
    } else {
        // 텍스트만 있는 쿼리
        currentMessage["parts"].push_back({{"text", currentQuery.text}});
    }
    
    contents.push_back(currentMessage);
    requestBody["contents"] = contents;
    
    // 4. Generation config 추가
    json generationConfig;
    generationConfig["temperature"] = 0.7;
    generationConfig["topK"] = 40;
    generationConfig["topP"] = 0.95;
    generationConfig["maxOutputTokens"] = 2048;
    
    // 이미지 생성이 필요한 경우 responseModalities 설정
    if (enableImageGeneration) {
        generationConfig["responseModalities"] = json::array({"TEXT", "IMAGE"});
    } else {
        generationConfig["responseModalities"] = json::array({"TEXT"});
    }
    
    requestBody["generationConfig"] = generationConfig;
    
    return requestBody;
}

// 이미지 요청인지 확인하는 함수
bool isImageGenerationRequest(const std::string& query) {
    std::vector<std::string> imageRequestPatterns = {
        "그려줘", "그림 그려줘", "이미지 그려줘", "이미지 생성해줘", 
        "draw", "generate image", "create picture"
    };
    
    for (const auto& pattern : imageRequestPatterns) {
        if (query.find(pattern) != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

int main() {
    // 1. System prompt 정의
    std::string systemPrompt = 
        "당신은 지식이 풍부하고 도움이 되는 AI 어시스턴트입니다. 다음 가이드라인을 따라 응답해주세요:\n\n"
        "1. 사용자가 한국어로 질문하면 한국어로, 영어로 질문하면 영어로 답변하세요.\n"
        "2. 사용자가 명시적으로 '그려줘', '그림 그려줘', '이미지 그려줘', '이미지 생성해줘'와 같은 표현을 사용하거나 "
        "이와 매우 유사한 표현을 사용할 때만 이미지를 포함한 응답을 제공하세요.\n"
        "3. 명확하고 정확한 정보를 제공하세요.\n"
        "4. 주어진 질문에 대해 최대한 도움이 되는 응답을 제공하세요.\n"
        "5. 이미지 생성 요청이 아닌 경우 항상 텍스트로만 응답하세요.";
    
    // 2. 이전 대화 내역 구성
    std::vector<Message> conversationHistory = {
        {
            "user", 
            "인공지능이란 무엇인가요?", 
            "", 
            ""
        },
        {
            "model",
            "인공지능(AI)은 인간의 학습, 추론, 자기 개선 능력 등을 모방하도록 설계된 시스템과 기계를 의미합니다. "
            "인공지능은 크게 약한 인공지능(특정 작업에 특화)과 강한 인공지능(인간 수준의 지능)으로 나눌 수 있습니다. "
            "현대 인공지능은 주로 머신러닝과 딥러닝 기술을 기반으로 하며, 데이터를 통해 학습하고 "
            "패턴을 인식하여 의사결정을 내립니다.",
            "",
            ""
        },
        {
            "user",
            "머신러닝의 종류에는 어떤 것들이 있나요?",
            "",
            ""
        },
        {
            "model",
            "머신러닝의 주요 종류는 다음과 같습니다:\n\n"
            "1. 지도학습(Supervised Learning): 레이블이 있는 데이터를 사용하여 입력과 출력 간의 관계를 학습합니다.\n"
            "2. 비지도학습(Unsupervised Learning): 레이블 없이 데이터의 패턴이나 구조를 발견합니다.\n"
            "3. 강화학습(Reinforcement Learning): 환경과의 상호작용을 통해 보상을 최대화하는 행동을 학습합니다.\n"
            "4. 준지도학습(Semi-supervised Learning): 소량의 레이블 데이터와 대량의 레이블이 없는 데이터를 함께 사용합니다.\n"
            "5. 자기지도학습(Self-supervised Learning): 데이터 자체에서 감독 신호를 생성하여 학습합니다.\n\n"
            "각 방식은 서로 다른 문제 유형에 적합합니다.",
            "",
            ""
        }
    };
    
    // 3. 현재 사용자 쿼리 정의
    Message currentQuery = {
        "user",
        "딥러닝과 일반 머신러닝의 차이점을 설명해주세요.",
        "",
        ""
    };
    
    // 4. 이미지 생성 요청인지 확인
    bool enableImageGeneration = isImageGenerationRequest(currentQuery.text);
    
    // 5. 요청 본문 생성
    json requestBody = createRequestBody(systemPrompt, conversationHistory, currentQuery, enableImageGeneration);
    
    // 6. 결과 출력 (실제로는 HTTP 요청을 보냄)
    std::cout << requestBody.dump(2) << std::endl;
    
    return 0;
}
