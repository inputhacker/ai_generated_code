#include "llm_client.hpp"
#include <curl/curl.h>
#include <thread>
#include <mutex>

// libcurl 응답을 위한 콜백 데이터 구조체
struct ResponseData {
    std::string data;
    LLMClient::ResponseCallback callback;
    bool isStreaming;
    bool* cancelRequested;
};

// libcurl 콜백 함수
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realSize = size * nmemb;
    ResponseData* resp = static_cast<ResponseData*>(userp);
    
    // 취소 요청 확인
    if (resp->cancelRequested && *resp->cancelRequested) {
        return 0; // 0 반환으로 curl_easy_perform 중단
    }
    
    resp->data.append(static_cast<char*>(contents), realSize);
    
    // 스트리밍 모드인 경우, 각 청크마다 콜백 호출
    if (resp->isStreaming && resp->callback) {
        try {
            // 스트리밍 응답 파싱 (이 예제에서는 단순화)
            Response streamResponse(resp->data);
            streamResponse.setStatus(ResponseStatus::PARTIAL);
            resp->callback(streamResponse);
            resp->data.clear();
        } catch (const std::exception& e) {
            // 파싱 오류는 무시하고 계속 데이터 수집
        }
    }
    
    return realSize;
}

LLMClient::LLMClient(const std::string& apiKey, const std::string& endpoint)
    : apiKey(apiKey), apiEndpoint(endpoint) {
    // libcurl 초기화
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

LLMClient::~LLMClient() {
    // 정리
    curl_global_cleanup();
}

Response LLMClient::sendQuery(const Query& query) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        Response errorResponse;
        errorResponse.setStatus(ResponseStatus::ERROR);
        errorResponse.setErrorMessage("Failed to initialize curl");
        return errorResponse;
    }
    
    std::string queryJson = query.serialize();
    ResponseData respData;
    respData.isStreaming = false;
    respData.cancelRequested = &cancelRequested;
    
    // 이전 취소 플래그 초기화
    cancelRequested = false;
    
    // 요청 시작 표시
    std::lock_guard<std::mutex> lock(std::mutex());
    requestInProgress = true;
    
    // curl 옵션 설정
    curl_easy_setopt(curl, CURLOPT_URL, apiEndpoint.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respData);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, queryJson.c_str());
    
    // HTTP 헤더 설정
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string authHeader = "Authorization: Bearer " + apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // 요청 수행
    CURLcode res = curl_easy_perform(curl);
    
    // 헤더 정리
    curl_slist_free_all(headers);
    
    // 요청 종료 표시
    requestInProgress = false;
    
    // 응답 처리
    Response response;
    
    if (res != CURLE_OK) {
        response.setStatus(ResponseStatus::ERROR);
        if (cancelRequested) {
            response.setErrorMessage("Request cancelled");
            response.setStatus(ResponseStatus::CANCELLED);
        } else {
            response.setErrorMessage(curl_easy_strerror(res));
        }
    } else {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        
        if (httpCode == 200) {
            try {
                response = Response::deserialize(respData.data);
                response.setStatus(ResponseStatus::SUCCESS);
            } catch (const std::exception& e) {
                response.setStatus(ResponseStatus::ERROR);
                response.setErrorMessage("Failed to parse response: " + std::string(e.what()));
            }
        } else if (httpCode == 429) {
            response.setStatus(ResponseStatus::THROTTLED);
            response.setErrorMessage("Rate limit exceeded");
        } else {
            response.setStatus(ResponseStatus::ERROR);
            response.setErrorMessage("HTTP error: " + std::to_string(httpCode));
        }
    }
    
    curl_easy_cleanup(curl);
    return response;
}

std::future<Response> LLMClient::sendQueryAsync(const Query& query) {
    return std::async(std::launch::async, [this, query]() {
        return this->sendQuery(query);
    });
}

bool LLMClient::streamQuery(const Query& query, ResponseCallback callback) {
    if (!isStreaming) {
        Response errorResponse;
        errorResponse.setStatus(ResponseStatus::ERROR);
        errorResponse.setErrorMessage("Streaming mode not enabled");
        callback(errorResponse);
        return false;
    }
    
    // 스트리밍 요청을 별도 스레드에서 처리
    std::thread([this, query, callback]() {
        CURL* curl = curl_easy_init();
        if (!curl) {
            Response errorResponse;
            errorResponse.setStatus(ResponseStatus::ERROR);
            errorResponse.setErrorMessage("Failed to initialize curl");
            callback(errorResponse);
            return;
        }
        
        std::string queryJson = query.serialize();
        ResponseData respData;
        respData.isStreaming = true;
        respData.callback = callback;
        respData.cancelRequested = &cancelRequested;
        
        // 이전 취소 플래그 초기화
        cancelRequested = false;
        
        // 요청 시작 표시
        {
            std::lock_guard<std::mutex> lock(std::mutex());
            requestInProgress = true;
        }
        
        // curl 옵션 설정
        std::string streamUrl = apiEndpoint;
        if (streamUrl.find("stream") == std::string::npos) {
            streamUrl += "/stream";
        }
        curl_easy_setopt(curl, CURLOPT_URL, streamUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respData);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeoutMs);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, queryJson.c_str());
        
        // HTTP 헤더 설정
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: text/event-stream");
        std::string authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        // 요청 수행
        CURLcode res = curl_easy_perform(curl);
        
        // 헤더 정리
        curl_slist_free_all(headers);
        
        // 요청 종료 표시
        {
            std::lock_guard<std::mutex> lock(std::mutex());
            requestInProgress = false;
        }
        
        // 최종 응답 처리
        Response finalResponse;
        
        if (res != CURLE_OK) {
            finalResponse.setStatus(ResponseStatus::ERROR);
            if (cancelRequested) {
                finalResponse.setErrorMessage("Stream cancelled");
                finalResponse.setStatus(ResponseStatus::CANCELLED);
            } else {
                finalResponse.setErrorMessage(curl_easy_strerror(res));
            }
        } else {
            finalResponse.setStatus(ResponseStatus::SUCCESS);
            finalResponse.getContent().addText("Stream completed");
        }
        
        callback(finalResponse);
        curl_easy_cleanup(curl);
    }).detach();
    
    return true;
}

bool LLMClient::cancelRequest() {
    if (!requestInProgress) {
        return false;
    }
    
    cancelRequested = true;
    return true;
}
