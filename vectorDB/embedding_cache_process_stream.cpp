private:
    // 임베딩 캐시
    std::unordered_map<std::string, std::vector<float>> embedding_cache;
    size_t max_cache_size = 1000; // 최대 캐시 크기

    // 임베딩 함수 업데이트 (캐싱 추가)
    std::vector<float> embed_text(const std::string& text) {
        // 캐시에서 확인
        auto it = embedding_cache.find(text);
        if (it != embedding_cache.end()) {
            return it->second;
        }
        
        // 캐시에 없으면 새로 계산
        std::vector<float> embedding = calculate_embedding(text);
        
        // 캐시가 최대 크기에 도달했으면 랜덤하게 하나 제거
        if (embedding_cache.size() >= max_cache_size) {
            auto it_to_delete = embedding_cache.begin();
            std::advance(it_to_delete, rand() % embedding_cache.size());
            embedding_cache.erase(it_to_delete);
        }
        
        // 캐시에 저장
        embedding_cache[text] = embedding;
        
        return embedding;
    }
    
    // 실제 임베딩 계산 함수
    std::vector<float> calculate_embedding(const std::string& text) {
        // 기존 임베딩 코드와 동일
        std::vector<std::string> tokens;
        std::vector<int> ids;
        tokenizer->Encode(text, &tokens);
        
        // 토큰 ID로 변환
        for (const auto& token : tokens) {
            ids.push_back(tokenizer->PieceToId(token));
        }
        
        // 간단한 임베딩 생성
        std::vector<float> embedding(embedding_dim, 0.0f);
        for (size_t i = 0; i < ids.size() && i < 512; i++) {
            for (int j = 0; j < embedding_dim; j++) {
                embedding[j] += std::sin(ids[i] * (j + 1.0f) / embedding_dim);
            }
        }
        
        return normalize_vector(embedding);
    }

public:
    // 캐시 크기 설정
    void set_cache_size(size_t size) {
        max_cache_size = size;
        if (embedding_cache.size() > max_cache_size) {
            size_t to_remove = embedding_cache.size() - max_cache_size;
            for (size_t i = 0; i < to_remove; i++) {
                auto it = embedding_cache.begin();
                embedding_cache.erase(it);
            }
        }
        std::cout << "임베딩 캐시 크기가 " << max_cache_size << "로 설정되었습니다." << std::endl;
    }
    
    // 스트리밍 방식으로 데이터 처리
    void process_stream(std::istream& input_stream, int batch_size = 10) {
        std::vector<std::string> batch_texts;
        std::string line;
        
        while (std::getline(input_stream, line)) {
            if (line.empty()) continue;
            
            batch_texts.push_back(line);
            
            if (batch_texts.size() >= batch_size) {
                add_texts_batch(batch_texts);
                batch_texts.clear();
            }
        }
        
        // 남은 배치 처리
        if (!batch_texts.empty()) {
            add_texts_batch(batch_texts);
        }
    }
