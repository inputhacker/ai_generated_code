// streamGenerateContent.cpp
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <regex>

using json = nlohmann::json;

// 스트리밍 응답 처리 함수
static size_t StreamCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::string chunk((char*)contents, realsize);
    
    // 스트림 데이터는 "[CHUNK]\n\n" 형태로 전송됨
    std::istringstream stream(chunk);
    std::string line;
    
    while (std::getline(stream, line)) {
        // 빈 줄 무시
        if (line.empty()) continue;
        
        // " " 로 시작하는 줄만 처리
        if (line.substr(0, 6) == " ") {
            std::string jsonData = line.substr(6); // " " 이후 부분만 추출
            
            // 스트림 종료 메시지 확인
            if (jsonData == "[DONE]") {
                std::cout << "스트리밍 완료" << std::endl;
                continue;
            }
            
            try {
                // JSON 파싱 및 텍스트 추출
                json responseChunk = json::parse(jsonData);
                
                if (responseChunk.contains("candidates") && !responseChunk["candidates"].empty() && 
                    responseChunk["candidates"][0].contains("content") && 
                    responseChunk["candidates"][0]["content"].contains("parts")) {
                    
                    for (const auto& part : responseChunk["candidates"][0]["content"]["parts"]) {
                        if (part.contains("text")) {
                            // 스트림 청크에서 응답 텍스트 출력 (줄바꿈 없이)
                            std::cout << part["text"].get<std::string>() << std::flush;
                        }
                    }
                }
            } catch (json::parse_error& e) {
                std::cerr << "청크 파싱 오류: " << e.what() << std::endl;
                std::cerr << "원본 데이터: " << jsonData << std::endl;
            }
        }
    }
    
    return realsize;
}

int main() {
    // 초기화
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL 초기화 실패" << std::endl;
        return 1;
    }

    std::string accessToken = "YOUR_ACCESS_TOKEN"; // gcloud auth print-access-token 으로 얻은 토큰
    std::string projectId = "YOUR_PROJECT_ID";
    std::string modelId = "gemini-2.0-flash";
    std::string url = "https://aiplatform.googleapis.com/v1/projects/" + projectId + 
                      "/locations/global/publishers/google/models/" + modelId + ":streamGenerateContent"; // URL 변경됨

    // 요청 데이터 생성 (동일함)
    json requestData = {
        {"contents", {
            {"role", "user"},
            {"parts", {{{"text", "안녕하세요, 오늘의 날씨를 알려주세요."}}}
        }},
        {"generation_config", {
            {"temperature", 0.9}
        }}
    };
    std::string requestBody = requestData.dump();

    // 헤더 및 요청 설정
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    // Server-Sent Events 형식을 처리하기 위한 헤더 추가
    headers = curl_slist_append(headers, "Accept: text/event-stream");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StreamCallback); // 콜백 함수 변경
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
    
    // 중요: 버퍼링을 비활성화하여 청크가 도착할 때마다 처리
    curl_easy_setopt(curl, CURLOPT_HTTP_TRANSFER_DECODING, 1L);

    std::cout << "Gemini 응답: ";
    
    // API 호출 실행
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "\ncurl_easy_perform() 실패: " << curl_easy_strerror(res) << std::endl;
    }

    std::cout << std::endl;  // 응답 후 줄바꿈 추가
    
    // 리소스 정리
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return 0;
}
