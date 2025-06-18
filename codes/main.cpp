#include "llm_client.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

// 파일에서 이미지 데이터 읽기 유틸리티 함수
std::vector<uint8_t> readImageFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    
    // 파일 크기 구하기
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // 데이터 읽기
    std::vector<uint8_t> data(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        throw std::runtime_error("Failed to read file: " + path);
    }
    
    return data;
}

int main() {
    // LLM 클라이언트 초기화
    LLMClient client("your_api_key_here", "https://api.gemini.google.com/v1/models/gemini-pro");
    client.setTimeout(60000);  // 60초 타임아웃
    
    try {
        // 예제 1: 단순 텍스트 쿼리
        std::cout << "===== 예제 1: 텍스트 쿼리 =====\n";
        Query textQuery("Explain quantum computing in simple terms");
        textQuery.setParameter("temperature", "0.7");
        
        Response textResponse = client.sendQuery(textQuery);
        if (textResponse.getStatus() == ResponseStatus::SUCCESS && textResponse.getContent().hasText()) {
            std::cout << "응답: " << textResponse.getContent().getText() << "\n\n";
        } else {
            std::cout << "오류: " << textResponse.getErrorMessage() << "\n\n";
        }
        
        // 예제 2: 이미지 경로를 포함한 쿼리
        std::cout << "===== 예제 2: 이미지 경로 쿼리 =====\n";
        Query imagePathQuery("What's in this image?");
        imagePathQuery.getContent().addImagePath("/path/to/image.jpg");
        
        Response imagePathResponse = client.sendQuery(imagePathQuery);
        if (imagePathResponse.getStatus() == ResponseStatus::SUCCESS && imagePathResponse.getContent().hasText()) {
            std::cout << "응답: " << imagePathResponse.getContent().getText() << "\n\n";
        } else {
            std::cout << "오류: " << imagePathResponse.getErrorMessage() << "\n\n";
        }
        
        // 예제 3: 이미지 데이터를 포함한 쿼리
        std::cout << "===== 예제 3: 이미지 데이터 쿼리 =====\n";
        Query imageDataQuery("Describe what you see in detail");
        try {
            std::vector<uint8_t> imageData = readImageFile("/path/to/another/image.jpg");
            imageDataQuery.getContent().addImageData(imageData, "image/jpeg");
            
            Response imageDataResponse = client.sendQuery(imageDataQuery);
            if (imageDataResponse.getStatus() == ResponseStatus::SUCCESS && imageDataResponse.getContent().hasText()) {
                std::cout << "응답: " << imageDataResponse.getContent().getText() << "\n\n";
            } else {
                std::cout << "오류: " << imageDataResponse.getErrorMessage() << "\n\n";
            }
        } catch (const std::exception& e) {
            std::cout << "이미지 로드 오류: " << e.what() << "\n\n";
        }
        
        // 예제 4: 비동기 쿼리
        std::cout << "===== 예제 4: 비동기 쿼리 =====\n";
        Query asyncQuery("Generate a short story about space exploration");
        asyncQuery.setParameter("max_tokens", "500");
        
        std::cout << "비동기 요청 시작...\n";
        std::future<Response> futureResponse = client.sendQueryAsync(asyncQuery);
        
        // 다른 작업을 수행하는 동안...
        std::cout << "다른 작업 수행 중...\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 결과 기다리기
        std::cout << "응답 기다리는 중...\n";
        
        // 최대 30초간 기다림
        if (futureResponse.wait_for(std::chrono::seconds(30)) == std::future_status::ready) {
            Response asyncResponse = futureResponse.get();
            if (asyncResponse.getStatus() == ResponseStatus::SUCCESS && asyncResponse.getContent().hasText()) {
                std::cout << "비동기 응답: " << asyncResponse.getContent().getText() << "\n\n";
            } else {
                std::cout << "비동기 오류: " << asyncResponse.getErrorMessage() << "\n\n";
            }
        } else {
            std::cout << "비동기 요청 시간 초과\n\n";
        }
        
        // 예제 5: 스트리밍 쿼리
        std::cout << "===== 예제 5: 스트리밍 쿼리 =====\n";
        Query streamQuery("Write a poem about artificial intelligence");
        client.setStreaming(true);
        
        bool complete = false;
        bool streamSuccess = client.streamQuery(streamQuery, [&complete](const Response& response) {
            if (response.getStatus() == ResponseStatus::PARTIAL) {
                // 부분 응답 출력
                if (response.getContent().hasText()) {
                    std::cout << response.getContent().getText();
                    std::cout.flush(); // 버퍼 즉시 출력
                }
            } else if (response.getStatus() == ResponseStatus::SUCCESS) {
                // 스트림 완료
                std::cout << "\n스트림 완료!\n";
                complete = true;
            } else if (response.getStatus() == ResponseStatus::ERROR) {
                std::cout << "\n스트리밍 오류: " << response.getErrorMessage() << std::endl;
                complete = true;
            }
        });
        
        if (!streamSuccess) {
            std::cout << "스트리밍 시작 실패\n";
        } else {
            // 스트림이 완료될 때까지 대기
            std::cout << "스트리밍 응답:\n";
            while (!complete) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            std::cout << "\n";
        }
        
        // 예제 6: 취소 가능한 쿼리
        std::cout << "===== 예제 6: 취소 가능한 쿼리 =====\n";
        Query longQuery("Generate an extremely detailed essay about the history of computing");
        
        // 별도 스레드에서 5초 후 취소 요청
        std::thread cancelThread([&client]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            std::cout << "\n\n5초 경과, 요청 취소 중...\n";
            bool cancelled = client.cancelRequest();
            std::cout << (cancelled ? "취소 요청 성공" : "취소 요청 실패") << std::endl;
        });
        
        std::cout << "오래 걸리는 요청 시작...\n";
        Response longResponse = client.sendQuery(longQuery);
        
        if (longResponse.getStatus() == ResponseStatus::CANCELLED) {
            std::cout << "요청이 취소되었습니다.\n";
        } else if (longResponse.getStatus() == ResponseStatus::SUCCESS) {
            std::cout << "응답: " << longResponse.getContent().getText() << "\n";
        } else {
            std::cout << "오류: " << longResponse.getErrorMessage() << "\n";
        }
        
        // 취소 스레드 종료 대기
        if (cancelThread.joinable()) {
            cancelThread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "예외 발생: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
