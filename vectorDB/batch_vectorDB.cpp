public:
    // 대량의 텍스트를 한번에 추가
    void add_texts_batch(const std::vector<std::string>& texts, 
                         const std::vector<std::string>& metadatas = {}) {
        if (!metadatas.empty() && metadatas.size() != texts.size()) {
            throw std::runtime_error("텍스트와 메타데이터의 개수가 일치해야 합니다.");
        }
        
        size_t start_id = stored_texts.size();
        if (start_id + texts.size() > max_elements) {
            throw std::runtime_error("최대 저장 용량을 초과합니다.");
        }
        
        std::cout << texts.size() << "개의 텍스트를 배치로 추가합니다..." << std::endl;
        
        // 먼저 모든 임베딩을 계산
        std::vector<std::vector<float>> embeddings(texts.size());
        
        #pragma omp parallel for
        for (size_t i = 0; i < texts.size(); i++) {
            embeddings[i] = embed_text(texts[i]);
        }
        
        // 인덱스에 추가
        for (size_t i = 0; i < texts.size(); i++) {
            index->addPoint(embeddings[i].data(), start_id + i);
            stored_texts.push_back(texts[i]);
            
            if (metadatas.empty()) {
                stored_metadata.push_back("");
            } else {
                stored_metadata.push_back(metadatas[i]);
            }
        }
        
        std::cout << "배치 추가 완료. 현재 저장된 항목 수: " << stored_texts.size() << std::endl;
    }
    
    // 대량의 쿼리를 한번에 처리
    std::vector<std::vector<std::pair<std::string, float>>> search_batch(
        const std::vector<std::string>& queries, 
        int k = 5,
        SimilarityType sim_type = COSINE) {
        
        std::vector<std::vector<std::pair<std::string, float>>> all_results;
        all_results.reserve(queries.size());
        
        std::cout << queries.size() << "개의 쿼리를 배치로 처리합니다..." << std::endl;
        
        #pragma omp parallel for
        for (size_t i = 0; i < queries.size(); i++) {
            std::vector<std::pair<std::string, float>> results = search(queries[i], k, sim_type);
            
            #pragma omp critical
            {
                all_results.push_back(results);
            }
        }
        
        std::cout << "배치 검색 완료." << std::endl;
        return all_results;
    }
