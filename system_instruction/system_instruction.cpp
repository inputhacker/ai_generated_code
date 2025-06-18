#include <curl/curl.h>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL 초기화 실패" << std::endl;
        return 1;
    }

    std::string accessToken = std::getenv("ACCESS_TOKEN");
    std::string projectId = std::getenv("PROJECT_ID");
    std::string modelId = std::getenv("MODEL_ID");
    std::string url = "https://aiplatform.googleapis.com/v1/projects/" + projectId +
                       "/locations/global/publishers/google/models/" + modelId + ":generateContent";

    std::cout << "accessToken=" << accessToken << std::endl;
    std::cout << "projectId=" << projectId << std::endl;
    std::cout << "modelId=" << modelId << std::endl;

    // contents 배열 직접 조작
    json contents = json::array();

    // system 메시지 생성
    json systemMsg;
    systemMsg["role"] = "system_instruction";
    systemMsg["parts"] = json::array();
    systemMsg["parts"].push_back({{"text", "당신은 전문 한국어 튜터입니다. 답변은 항상 존댓말로 하고, 문법적으로 정확한 한국어를 사용하세요. 각 응답 끝에는 '오늘도 좋은 하루 되세요!'를 추가하세요."}});
//contents.push_back(systemMsg);

    // user 메시지 생성
    json userMsg;
    userMsg["role"] = "user";
    userMsg["parts"] = json::array();
    userMsg["parts"].push_back({{"text", "한국어로 인사하는 방법 알려줘"}});
    contents.push_back(userMsg);

    // 최종 요청 데이터
    json requestData;
    requestData["system_instruction"] = systemMsg;
    requestData["contents"] = contents;
    requestData["generation_config"] = {
        {"temperature", 0.2},
        {"top_p", 0.8},
        {"top_k", 40}
    };
    std::string requestBody = requestData.dump(4);

    std::cout << "requestBody=" << requestBody << std::endl;

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + accessToken).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() 실패: " << curl_easy_strerror(res) << std::endl;
    } else {

    	std::cout << "[RESPONSE] : " << response << std::endl;
        try {
            json responseJson = json::parse(response);
            if (responseJson.contains("candidates") && !responseJson["candidates"].empty() &&
                responseJson["candidates"].contains("content") &&
                responseJson["candidates"]["content"].contains("parts")) {

                for (const auto& part : responseJson["candidates"]["content"]["parts"]) {
                    if (part.contains("text")) {
                        std::cout << "Gemini 응답: " << part["text"].get<std::string>() << std::endl;
                    }
                }
            }
        } catch (json::parse_error& e) {
            std::cerr << "JSON 파싱 오류: " << e.what() << std::endl;
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return 0;
}
