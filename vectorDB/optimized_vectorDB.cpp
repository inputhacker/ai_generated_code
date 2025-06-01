// VectorDB 클래스에 추가할 메서드
public:
    // 메모리 사용량을 줄이기 위한 벡터 양자화 기능
    void quantize(int bits = 8) {
        if (bits != 8 && bits != 4) {
            throw std::runtime_error("지원하는 양자화 비트 수는 4 또는 8입니다.");
        }

        std::cout << "기존 벡터를 " << bits << "비트로 양자화합니다..." << std::endl;
        
        // 벡터를 다시 로드하여 양자화된 형태로 저장
        for (size_t i = 0; i < stored_texts.size(); i++) {
            // 기존 임베딩 가져오기
            std::vector<float> embedding = embed_text(stored_texts[i]);
            
            // 임베딩 양자화 (간단한 구현)
            if (bits == 8) {
                // 8비트 양자화: -128 ~ 127 범위로 매핑
                std::vector<int8_t> quantized(vector_dimension);
                for (size_t j = 0; j < vector_dimension; j++) {
                    quantized[j] = static_cast<int8_t>(embedding[j] * 127.0f);
                }
                
                // 다시 float로 변환하여 인덱스에 저장
                std::vector<float> dequantized(vector_dimension);
                for (size_t j = 0; j < vector_dimension; j++) {
                    dequantized[j] = quantized[j] / 127.0f;
                }
                
                // 기존 벡터 업데이트
                index->markDeleted(i);
                index->addPoint(dequantized.data(), i);
            } 
            else if (bits == 4) {
                // 4비트 양자화: 각 바이트에 2개의 값을 저장
                std::vector<uint8_t> quantized((vector_dimension + 1) / 2);
                for (size_t j = 0; j < vector_dimension; j += 2) {
                    int8_t high = static_cast<int8_t>(embedding[j] * 7.0f);
                    int8_t low = (j + 1 < vector_dimension) ? 
                                static_cast<int8_t>(embedding[j + 1] * 7.0f) : 0;
                    
                    // 4비트씩 패킹
                    quantized[j / 2] = (high & 0x0F) << 4 | (low & 0x0F);
                }
                
                // 다시 float로 변환
                std::vector<float> dequantized(vector_dimension);
                for (size_t j = 0; j < vector_dimension; j += 2) {
                    dequantized[j] = ((quantized[j / 2] >> 4) & 0x0F) / 7.0f;
                    if (j + 1 < vector_dimension) {
                        dequantized[j + 1] = (quantized[j / 2] & 0x0F) / 7.0f;
                    }
                }
                
                // 기존 벡터 업데이트
                index->markDeleted(i);
                index->addPoint(dequantized.data(), i);
            }
        }
        
        std::cout << "양자화 완료. 메모리 사용량이 감소했습니다." << std::endl;
    }
    
    // 메모리 사용량 보고
    void report_memory_usage() {
        size_t index_size = index->cal_size();
        size_t texts_size = 0;
        size_t metadata_size = 0;
        
        for (const auto& text : stored_texts) {
            texts_size += text.size();
        }
        
        for (const auto& meta : stored_metadata) {
            metadata_size += meta.size();
        }
        
        std::cout << "메모리 사용 보고:" << std::endl;
        std::cout << "- 인덱스 크기: " << (index_size / 1024.0 / 1024.0) << " MB" << std::endl;
        std::cout << "- 텍스트 데이터: " << (texts_size / 1024.0) << " KB" << std::endl;
        std::cout << "- 메타데이터: " << (metadata_size / 1024.0) << " KB" << std::endl;
        std::cout << "- 총 메모리 사용량: " << ((index_size + texts_size + metadata_size) / 1024.0 / 1024.0) << " MB" << std::endl;
    }
