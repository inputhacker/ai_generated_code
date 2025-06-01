int main() {
    try {
        // 스레드 풀 초기화 (라즈베리파이 코어 수에 맞게 조정)
        ThreadPool pool(4);
        
        std::cout << "== 라즈베리파이용 VectorDB 데모 ==" << std::endl;
        
        // 분산 VectorDB 초기화
        DistributedVectorDB db(128, 1000, 3);
        
        // 샘플 텍스트 데이터
        std::vector<std::string> texts = {
            "라즈베리파이는 저비용 소형 컴퓨터입니다.",
            "벡터 데이터베이스는 임베딩을 저장하고 검색하는데 사용됩니다.",
            "C++은 성능이 중요한 응용 프로그램에 적합합니다.",
            "임베딩은 텍스트나 이미지를 벡터로 변환하는 과정입니다.",
            "코사인 유사도는 두 벡터 간의 각도를 측정하는 방법입니다."
        };
        
        // 배치 처리로 텍스트 추가
        db.add_texts_batch(texts);
        
        // 메모리 사용량 출력 (실제로는 각 샤드 별로 확인 필요)
        std::cout << "데이터 추가 완료" << std::endl;
        
        // 비동기 쿼리 처리
        std::vector<std::future<std::vector<std::pair<std::string, float>>>> futures;
        
        std::vector<std::string> queries = {
            "벡터 유사도란 무엇인가요?",
            "라즈베리파이의 특징은 무엇인가요?",
            "임베딩 기술의 원리는 무엇인가요?"
        };
        
        // 비동기로 모든 쿼리 실행
        for (const auto& query : queries) {
            futures.push_back(
                pool.enqueue([&db, query]() {
                    return db.search(query, 3, VectorDB::COSINE);
                })
            );
        }
        
        // 결과 수집 및 출력
        for (size_t i = 0; i < queries.size(); i++) {
            auto results = futures[i].get();
            
            std::cout << "\n쿼리: " << queries[i] << std::endl;
            std::cout << "결과:" << std::endl;
            
            for (const auto& result : results) {
                std::cout << "- " << result.first << "\n  유사도: " << result.second << std::endl;
            }
        }
        
        // 데이터베이스 저장
        db.save("distributed_db");
        std::cout << "\n데이터베이스가 저장되었습니다." << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "오류 발생: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
