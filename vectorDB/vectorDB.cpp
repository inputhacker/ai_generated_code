#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <hnswlib/hnswlib.h>
#include <nlohmann/json.hpp>
#include <sentencepiece_processor.h>

using json = nlohmann::json;

class VectorDB {
private:
    // HNSW 인덱스 관련 변수
    hnswlib::HierarchicalNSW<float>* index;
    int vector_dimension;
    int max_elements;
    
    // 임베딩 모델 관련 변수
    sentencepiece::SentencePieceProcessor* tokenizer;
    std::vector<float> model_weights;
    int embedding_dim;
    
    // 메타데이터 저장
    std::vector<std::string> stored_texts;
    std::vector<std::string> stored_metadata;

    // 유틸리티 함수
    std::vector<float> normalize_vector(const std::vector<float>& vec) {
        float sum = 0.0f;
        for (const auto& v : vec) {
            sum += v * v;
        }
        float norm = std::sqrt(sum);
        
        std::vector<float> normalized(vec.size());
        for (size_t i = 0; i < vec.size(); i++) {
            normalized[i] = vec[i] / norm;
        }
        return normalized;
    }
    
    // 간단한 임베딩 모델 (실제로는 외부 라이브러리 사용 권장)
    std::vector<float> embed_text(const std::string& text) {
        // 실제 구현에서는 여기에 임베딩 모델을 연결해야 합니다
        // 이 예제에서는 간단한 임베딩 함수로 대체합니다
        
        std::vector<std::string> tokens;
        std::vector<int> ids;
        tokenizer->Encode(text, &tokens);
        
        // 토큰 ID로 변환
        for (const auto& token : tokens) {
            ids.push_back(tokenizer->PieceToId(token));
        }
        
        // 간단한 임베딩 생성 (실제 구현에서는 모델 가중치 사용)
        std::vector<float> embedding(embedding_dim, 0.0f);
        for (size_t i = 0; i < ids.size() && i < 512; i++) {
            for (int j = 0; j < embedding_dim; j++) {
                // 실제로는 여기서 모델의 가중치를 사용해야 합니다
                embedding[j] += std::sin(ids[i] * (j + 1.0f) / embedding_dim);
            }
        }
        
        return normalize_vector(embedding);
    }

public:
    enum SimilarityType {
        COSINE,
        DOT_PRODUCT,
        EUCLIDEAN,
        MANHATTAN
    };

    VectorDB(int dim = 384, int max_elems = 10000, const std::string& space = "cosine") 
        : vector_dimension(dim), max_elements(max_elems), embedding_dim(dim) {
        
        // HNSW 인덱스 초기화
        std::string space_type = space;
        if (space_type == "cosine") {
            index = new hnswlib::HierarchicalNSW<float>(hnswlib::InnerProductSpace(dim), max_elements);
        } else if (space_type == "l2") {
            index = new hnswlib::HierarchicalNSW<float>(hnswlib::L2Space(dim), max_elements);
        } else {
            throw std::runtime_error("지원되지 않는 거리 측정 방식입니다. 'cosine' 또는 'l2'를 사용하세요.");
        }
        
        // 토크나이저 초기화 (실제로는 모델 파일 경로 지정 필요)
        tokenizer = new sentencepiece::SentencePieceProcessor();
        // tokenizer->Load("path/to/tokenizer.model");
        
        std::cout << "VectorDB가 초기화되었습니다. 차원: " << dim << ", 최대 요소 수: " << max_elems << std::endl;
    }
    
    ~VectorDB() {
        delete index;
        delete tokenizer;
    }
    
    // 텍스트를 임베딩하여 데이터베이스에 추가
    void add_text(const std::string& text, const std::string& metadata = "") {
        // 현재 저장된 요소의 개수 확인
        size_t current_id = stored_texts.size();
        if (current_id >= max_elements) {
            throw std::runtime_error("최대 저장 용량에 도달했습니다.");
        }
        
        // 텍스트 임베딩
        std::vector<float> embedding = embed_text(text);
        
        // 인덱스에 추가
        index->addPoint(embedding.data(), current_id);
        
        // 원본 텍스트와 메타데이터 저장
        stored_texts.push_back(text);
        stored_metadata.push_back(metadata);
        
        std::cout << "ID " << current_id << "로 텍스트가 추가되었습니다." << std::endl;
    }
    
    // 사용자 쿼리에 가장 유사한 텍스트 검색
    std::vector<std::pair<std::string, float>> search(const std::string& query, int k = 5, 
                                                     SimilarityType sim_type = COSINE) {
        // 쿼리 임베딩
        std::vector<float> query_embedding = embed_text(query);
        
        // HNSW 라이브러리로 검색 (기본 코사인 유사도)
        std::priority_queue<std::pair<float, size_t>> results;
        
        if (sim_type == COSINE || sim_type == DOT_PRODUCT) {
            // HNSW 내장 검색 사용 (코사인이나 닷 프로덕트)
            std::vector<float> distances;
            std::vector<hnswlib::labeltype> labels;
            index->searchKnn(query_embedding.data(), k, &labels, &distances);
            
            // 결과 변환
            for (size_t i = 0; i < labels.size(); i++) {
                float score = 0;
                if (sim_type == COSINE) {
                    // 코사인 유사도 = 1 - 거리
                    score = 1.0f - distances[i];
                } else {
                    // 닷 프로덕트
                    score = -distances[i]; // HNSW에서는 거리가 음수로 저장됨
                }
                results.push(std::make_pair(score, labels[i]));
            }
        } else {
            // 유클리드 또는 맨해튼 거리는 모든 벡터와 직접 계산
            for (size_t i = 0; i < stored_texts.size(); i++) {
                std::vector<float> stored_embedding = embed_text(stored_texts[i]);
                float distance = 0.0f;
                
                if (sim_type == EUCLIDEAN) {
                    // 유클리드 거리 계산
                    for (size_t j = 0; j < vector_dimension; j++) {
                        float diff = query_embedding[j] - stored_embedding[j];
                        distance += diff * diff;
                    }
                    distance = std::sqrt(distance);
                    results.push(std::make_pair(-distance, i)); // 거리가 작을수록 유사도가 높음
                } else if (sim_type == MANHATTAN) {
                    // 맨해튼 거리 계산
                    for (size_t j = 0; j < vector_dimension; j++) {
                        distance += std::abs(query_embedding[j] - stored_embedding[j]);
                    }
                    results.push(std::make_pair(-distance, i)); // 거리가 작을수록 유사도가 높음
                }
            }
        }
        
        // 결과 변환
        std::vector<std::pair<std::string, float>> final_results;
        while (!results.empty() && final_results.size() < k) {
            auto top = results.top();
            results.pop();
            
            final_results.push_back(std::make_pair(
                stored_texts[top.second] + 
                (stored_metadata[top.second].empty() ? "" : " [" + stored_metadata[top.second] + "]"), 
                top.first
            ));
        }
        
        // 점수 기준으로 내림차순 정렬 (가장 유사한 것이 먼저 오도록)
        std::sort(final_results.begin(), final_results.end(), 
            [](const auto& a, const auto& b) { return a.second > b.second; });
            
        return final_results;
    }
    
    // 데이터베이스 저장
    bool save(const std::string& path) {
        try {
            // 인덱스 저장
            index->saveIndex(path + ".index");
            
            // 메타데이터 및 텍스트 저장
            json metadata;
            metadata["dimension"] = vector_dimension;
            metadata["max_elements"] = max_elements;
            metadata["texts"] = stored_texts;
            metadata["metadata"] = stored_metadata;
            
            std::ofstream file(path + ".json");
            file << metadata.dump(4);
            file.close();
            
            std::cout << "데이터베이스가 " << path << "에 저장되었습니다." << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "저장 중 오류 발생: " << e.what() << std::endl;
            return false;
        }
    }
    
    // 데이터베이스 로드
    bool load(const std::string& path) {
        try {
            // 메타데이터 및 텍스트 로드
            std::ifstream file(path + ".json");
            json metadata;
            file >> metadata;
            file.close();
            
            vector_dimension = metadata["dimension"];
            max_elements = metadata["max_elements"];
            stored_texts = metadata["texts"].get<std::vector<std::string>>();
            stored_metadata = metadata["metadata"].get<std::vector<std::string>>();
            
            // 인덱스 재생성 및 로드
            delete index;
            index = new hnswlib::HierarchicalNSW<float>(hnswlib::InnerProductSpace(vector_dimension), max_elements);
            index->loadIndex(path + ".index", max_elements);
            
            std::cout << "데이터베이스가 " << path << "에서 로드되었습니다." << std::endl;
            std::cout << "저장된 항목 수: " << stored_texts.size() << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "로드 중 오류 발생: " << e.what() << std::endl;
            return false;
        }
    }
};
