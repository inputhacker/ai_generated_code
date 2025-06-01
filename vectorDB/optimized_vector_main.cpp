int main() {
    // VectorDB 초기화 (임베딩 차원 128, 최대 5000개 항목)
    VectorDB db(128, 5000);
    
    // 캐시 크기 설정
    db.set_cache_size(500);
    
    // 배치로 데이터 추가
    std::vector<std::string> texts = {
        "라즈베리파이는 저비용 소형 컴퓨터입니다.",
        "벡터 데이터베이스는 임베딩을 저장하고 검색하는데 사용됩니다.",
        "C++은 성능이 중요한 응용 프로그램에 적합합니다.",
        "임베딩은 텍스트나 이미지를 벡터로 변환하는 과정입니다.",
        "코사인 유사도는 두 벡터 간의 각도를 측정하는 방법입니다.",
        "라즈베리파이 5는 이전 모델보다 성능이 크게 향상되었습니다.",
        "임베디드 시스템은 특정 기능을 수행하는 전용 컴퓨터 시스템입니다.",
        "벡터 검색은 대용량 데이터에서 유사한 항목을 빠르게 찾는 방법입니다.",
        "C++17은 많은 유용한 기능을 추가한 C++ 표준입니다.",
        "자연어 처리는 컴퓨터가 인간의 언어를 이해하고 처리하는 기술입니다."
    };
    
    std::vector<std::string> metadatas = {
        "하드웨어", "데이터베이스", "프로그래밍", "머신러닝", "수학",
        "하드웨어", "임베디드", "알고리즘", "프로그래밍", "NLP"
    };
    
    // 배치 추가
    db.add_texts_batch(texts, metadatas);
    
    // 메모리 사용량 보고
    db.report_memory_usage();
    
    // 양자화하여 메모리 사용 최적화
    db.quantize(8);
    
    // 양자화 후 메모리 사용량 보고
    db.report_memory_usage();
    
    // 배치 검색
    std::vector<std::string> queries = {
        "벡터 간의 유사도를 측정하는 방법은 무엇이 있나요?",
        "라즈베리파이에서 프로그래밍하는 방법",
        "임베딩 모델의 원리는 무엇인가요?"
    };
    
    std::vector<std::vector<std::pair<std::string, float>>> results = db.search_batch(queries, 3);
    
    // 결과 출력
    for (size_t i = 0; i < results.size(); i++) {
        std::cout << "\n쿼리: " << queries[i] << std::endl;
        std::cout << "결과:" << std::endl;
        
        for (const auto& result : results[i]) {
            std::cout << "- " << result.first << "\n  유사도: " << result.second << std::endl;
        }
    }
    
    // 데이터베이스 저장
    db.save("optimized_vectordb");
    
    // 파일에서 스트리밍 방식으로 데이터 로드 예시
    std::ifstream input_file("sample_texts.txt");
    if (input_file.is_open()) {
        db.process_stream(input_file, 20);  // 20개씩 배치로 처리
        input_file.close();
    }
    
    return 0;
}
