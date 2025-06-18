// generateContent.cpp
#include <curl/curl.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// CURL 콜백 함수
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
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
                       "/locations/global/publishers/google/models/" + modelId + ":generateContent";

    // 요청 데이터 생성
    json requestData = {
        {"contents", {
            {"role", "user"},
            {"parts", {{{"text", "안녕하세요, 오늘의 날씨를 알려주세요."}}}
        }}
    };
    std::string requestBody = requestData.dump();

    // 헤더 및 요청 설정
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string response;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // API 호출 
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() 실패: " << curl_easy_strerror(res) << std::endl;
    } else {
        // 응답 JSON 파싱
        try {
            json responseJson = json::parse(response);
            
            // 응답에서 텍스트 추출
            if (responseJson.contains("candidates") && !responseJson["candidates"].empty() && 
                responseJson["candidates"][0].contains("content") && 
                responseJson["candidates"][0]["content"].contains("parts")) {
                
                for (const auto& part : responseJson["candidates"][0]["content"]["parts"]) {
                    if (part.contains("text")) {
                        std::cout << "Gemini 응답: " << part["text"].get<std::string>() << std::endl;
                    }
                }
            }
        } catch (json::parse_error& e) {
            std::cerr << "JSON 파싱 오류: " << e.what() << std::endl;
        }
    }

    // 리소스 정리
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    return 0;
}
