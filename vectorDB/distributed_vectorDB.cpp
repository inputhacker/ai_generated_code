class DistributedVectorDB {
private:
    std::vector<VectorDB*> shards;
    int shard_count;
    int dimension;
    
public:
    DistributedVectorDB(int dim = 384, int max_elems_per_shard = 1000, int num_shards = 3) 
        : shard_count(num_shards), dimension(dim) {
        
        std::cout << num_shards << "개의 샤드로 분산 VectorDB를 초기화합니다." << std::endl;
        
        for (int i = 0; i < num_shards; i++) {
            shards.push_back(new VectorDB(dim, max_elems_per_shard));
            std::cout << "샤드 " << i << " 초기화됨" << std::endl;
        }
    }
    
    ~DistributedVectorDB() {
        for (auto shard : shards) {
            delete shard;
        }
    }
    
    // 텍스트 추가 - 간단한 해싱으로 샤드 선택
    void add_text(const std::string& text, const std::string& metadata = "") {
        // 간단한 해시 함수로 샤드 선택
        size_t hash_val = std::hash<std::string>{}(text);
        int shard_idx = hash_val % shard_count;
        
        // 선택된 샤드에 추가
        shards[shard_idx]->add_text(text, metadata);
    }
    
    // 배치 추가
    void add_texts_batch(const std::vector<std::string>& texts, 
                         const std::vector<std::string>& metadatas = {}) {
        for (size_t i = 0; i < texts.size(); i++) {
            std::string meta = metadatas.size() > i ? metadatas[i] : "";
            add_text(texts[i], meta);
        }
    }
    
    // 검색 - 모든 샤드에서 검색 후 결과 병합
    std::vector<std::pair<std::string, float>> search(const std::string& query, 
                                                     int k = 5, 
                                                     VectorDB::SimilarityType sim_type = VectorDB::COSINE) {
        // 각 샤드에서 k개씩 결과를 가져옴
        std::vector<std::vector<std::pair<std::string, float>>> shard_results;
        
        for (auto shard : shards) {
            shard_results.push_back(shard->search(query, k, sim_type));
        }
        
        // 결과 병합 및 정렬
        std::vector<std::pair<std::string, float>> merged_results;
        for (const auto& results : shard_results) {
            merged_results.insert(merged_results.end(), results.begin(), results.end());
        }
        
        // 점수로 정렬
        std::sort(merged_results.begin(), merged_results.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // 상위 k개만 반환
        if (merged_results.size() > k) {
            merged_results.resize(k);
        }
        
        return merged_results;
    }
    
    // 저장 및 로드 기능
    bool save(const std::string& base_path) {
        for (int i = 0; i < shard_count; i++) {
            std::string shard_path = base_path + "_shard" + std::to_string(i);
            if (!shards[i]->save(shard_path)) {
                return false;
            }
        }
        return true;
    }
    
    bool load(const std::string& base_path, int num_shards) {
        // 기존 샤드 제거
        for (auto shard : shards) {
            delete shard;
        }
        shards.clear();
        
        // 새 샤드 로드
        shard_count = num_shards;
        for (int i = 0; i < num_shards; i++) {
            std::string shard_path = base_path + "_shard" + std::to_string(i);
            
            VectorDB* new_shard = new VectorDB();
            if (!new_shard->load(shard_path)) {
                delete new_shard;
                return false;
            }
            
            shards.push_back(new_shard);
        }
        return true;
    }
};
