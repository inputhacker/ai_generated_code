// VectorDB 클래스에 추가할 필드와 메서드
private:
    bool use_mmap = false;
    std::string mmap_file;
    
public:
    // 메모리 매핑 모드로 전환
    void enable_memory_mapping(const std::string& file_path) {
        if (stored_texts.size() > 0) {
            std::cout << "경고: 기존 데이터가 있는 상태에서 메모리 매핑을 활성화하면 데이터가 디스크로 옮겨집니다." << std::endl;
        }
        
        use_mmap = true;
        mmap_file = file_path;
        
        // 메모리 매핑된 인덱스 생성
        delete index;
        index = new hnswlib::HierarchicalNSW<float>(
            hnswlib::InnerProductSpace(vector_dimension), 
            max_elements, 
            16,   // M 파라미터
            200,  // ef_construction
            mmap_file
        );
        
        std::cout << "메모리 매핑이 활성화되었습니다. 파일: " << mmap_file << std::endl;
        
        // 기존 데이터 다시 추가 (필요한 경우)
        if (stored_texts.size() > 0) {
            std::vector<std::string> temp_texts = stored_texts;
            std::vector<std::string> temp_meta = stored_metadata;
            
            stored_texts.clear();
            stored_metadata.clear();
            
            for (size_t i = 0; i < temp_texts.size(); i++) {
                add_text(temp_texts[i], temp_meta[i]);
            }
        }
    }
