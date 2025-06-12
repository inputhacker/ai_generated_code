// 이미지 생성 요청 예시
void imageGenerationExample() {
    // System prompt와 대화 내역은 동일하게 유지
    std::string systemPrompt = "..."; // 위와 동일
    std::vector<Message> conversationHistory = {...}; // 위와 동일
    
    // 이미지 생성 요청 쿼리
    Message drawRequest = {
        "user",
        "바다를 배경으로 노을이 지는 아름다운 풍경을 그려줘",
        "",
        ""
    };
    
    // 이미지 생성 요청인지 확인 (이 경우 true)
    bool enableImageGeneration = isImageGenerationRequest(drawRequest.text);
    
    // 요청 본문 생성
    json requestBody = createRequestBody(systemPrompt, conversationHistory, drawRequest, enableImageGeneration);
    
    // 결과 출력
    std::cout << requestBody.dump(2) << std::endl;
}
