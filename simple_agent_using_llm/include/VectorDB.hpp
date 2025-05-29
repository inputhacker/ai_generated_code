#ifndef VECTOR_DB_HPP
#define VECTOR_DB_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>

// Vector DB 인터페이스 클래스
class VectorDB {
public:
    virtual ~VectorDB() = default;
    virtual void store(const std::string& id, const std::vector<float>& embedding, 
                      const std::string& metadata) = 0;
    virtual std::vector<std::pair<std::string, float>> search(
        const std::vector<float>& queryEmbedding, int topK) = 0;
};

// 간단한 Vector DB 구현
class SimpleVectorDB : public VectorDB {
private:
    struct Entry {
        std::vector<float> embedding;
        std::string metadata;
    };
    
    std::unordered_map<std::string, Entry> database;
    
    float cosineSimilarity(const std::vector<float>& vec1, const std::vector<float>& vec2);
    
public:
    void store(const std::string& id, const std::vector<float>& embedding, 
              const std::string& metadata) override;
              
    std::vector<std::pair<std::string, float>> search(
        const std::vector<float>& queryEmbedding, int topK) override;
    
    void persistToDisk();
    void loadFromDisk();
};

#endif // VECTOR_DB_HPP
