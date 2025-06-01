int main() {
    // VectorDB 초기화 (384차원, 최대 1000개 요소)
    VectorDB db(384, 1000);
    
    // 텍스트 추가
    db.add_text("라즈베리파이는 저비용 소형 컴퓨터입니다.", "하드웨어");
    db.add_text("벡터 데이터베이스는 임베딩을 저장하고 검색하는데 사용됩니다.", "데이터베이스");
    db.add_text("C++은 성능이 중요한 응용 프로그램에 적합합니다.", "프로그래밍");
    db.add_text("임베딩은 텍스트나 이미지를 벡터로 변환하는 과정입니다.", "머신러닝");
    db.add_text("코사인 유사도는 두 벡터 간의 각도를 측정하는 방법입니다.", "수학");
    
    // 검색 기능 테스트 (코사인 유사도)
    std::cout << "\n코사인 유사도 검색:" << std::endl;
    auto results = db.search("벡터 간의 유사도를 측정하는 방법은 무엇이 있나요?", 3, VectorDB::COSINE);
    
    for (const auto& result : results) {
        std::cout << "텍스트: " << result.first << "\n유사도: " << result.second << std::endl;
    }
    
    // 유클리드 거리로 검색
    std::cout << "\n유클리드 거리 검색:" << std::endl;
    results = db.search("라즈베리파이에서 프로그래밍", 3, VectorDB::EUCLIDEAN);
    
    for (const auto& result : results) {
        std::cout << "텍스트: " << result.first << "\n점수: " << result.second << std::endl;
    }
    
    // 데이터베이스 저장
    db.save("vectordb");
    
    // 데이터베이스 로드 테스트
    VectorDB new_db;
    new_db.load("vectordb");
    
    // 로드된 데이터베이스로 검색
    std::cout << "\n로드된 데이터베이스로 검색:" << std::endl;
    results = new_db.search("벡터와 임베딩", 2, VectorDB::MANHATTAN);
    
    for (const auto& result : results) {
        std::cout << "텍스트: " << result.first << "\n점수: " << result.second << std::endl;
    }
    
    return 0;
}
