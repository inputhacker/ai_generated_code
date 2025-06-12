// 이미지를 포함한 요청 예시
void imageRequestExample() {
    // System prompt와 대화 내역은 동일하게 유지
    std::string systemPrompt = "..."; // 위와 동일
    std::vector<Message> conversationHistory = {...}; // 위와 동일
    
    // 이미지를 포함한 현재 쿼리
    Message imageQuery = {
        "user",
        "이 이미지에 있는 동물은 무엇인가요?",
        "image/jpeg",
        "BASE64_ENCODED_IMAGE_DATA" // 실제 base64로 인코딩된 이미지 데이터
    };
    
    // 요청 본문 생성
    json requestBody = createRequestBody(systemPrompt, conversationHistory, imageQuery, false);
    
    // 결과 출력
    std::cout << requestBody.dump(2) << std::endl;
}
