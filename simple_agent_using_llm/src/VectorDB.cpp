#include "VectorDB.hpp"
#include <algorithm>
#include <cmath>

float SimpleVectorDB::cosineSimilarity(const std::vector<float>& vec1, const std::vector<float>& vec2) {
    float dotProduct = 0.0f;
    float norm1 = 0.0f;
    float norm2 = 0.0f;
    
    for (size_t i = 0; i < vec1.size() && i < vec2.size(); i++) {
        dotProduct += vec1[i] * vec2[i];
        norm1 += vec1[i] * vec1[i];
        norm2 += vec2[i] * vec2[i];
    }
    
    if (norm1 == 0.0f || norm2 == 0.0f) return 0.0f;
    
    return dotProduct / (std::sqrt(norm1) * std::sqrt(norm2));
}

void SimpleVectorDB::store(const std::string& id, const std::vector<float>& embedding, 
                          const std::string& metadata) {
    database[id] = {embedding, metadata};
    persistToDisk();
}

std::vector<std::pair<std::string, float>> SimpleVectorDB::search(
    const std::vector<float>& queryEmbedding, int topK) {
    
    std::vector<std::pair<std::string, float>> results;
    
    for (const auto& entry : database) {
        float similarity = cosineSimilarity(queryEmbedding, entry.second.embedding);
        results.push_back({entry.second.metadata, similarity});
    }
    
    std::sort(results.begin(), results.end(), 
             [](const auto& a, const auto& b) { return a.second > b.second; });
    
    if (results.size() > static_cast<size_t>(topK)) {
        results.resize(topK);
    }
    
    return results;
}

void SimpleVectorDB::persistToDisk() {
    std::ofstream file("vector_db.data");
    // 실제 저장 로직은 별도로 구현 필요
    if (file.is_open()) {
        for (const auto& entry : database) {
            // 간단한 형식으로만 저장
            file << entry.first << "\n";
            // 실제 구현에서는 임베딩과 메타데이터도 저장해야 함
        }
    }
}

void SimpleVectorDB::loadFromDisk() {
    std::ifstream file("vector_db.data");
    // 실제 로드 로직은 별도로 구현 필요
    if (file.is_open()) {
        // 데이터 로드 로직
    }
}
