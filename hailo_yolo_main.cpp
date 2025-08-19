#include "hailo_yolo11_seg.hpp"
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "사용법: " << argv[0] << " <hef_file> <image_file>" << std::endl;
        return -1;
    }
    
    std::string hef_path = argv[1];
    std::string image_path = argv[2];
    
    // YOLO11 Segmentation 모델 초기화
    HailoYOLO11Seg model(hef_path);
    
    auto status = model.initialize();
    if (status != HAILO_SUCCESS) {
        std::cerr << "모델 초기화 실패: " << status << std::endl;
        return -1;
    }
    
    // 추론 실행
    auto start_time = std::chrono::high_resolution_clock::now();
    auto detections = model.predict(image_path);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "추론 시간: " << duration.count() << "ms" << std::endl;
    
    // 결과 출력
    std::cout << "검출된 객체 수: " << detections.size() << std::endl;
    
    for (size_t i = 0; i < detections.size(); i++) {
        const auto& det = detections[i];
        std::cout << "객체 " << i + 1 << ":" << std::endl;
        std::cout << "  클래스 ID: " << det.class_id << std::endl;
        std::cout << "  신뢰도: " << det.confidence << std::endl;
        std::cout << "  바운딩 박스: (" << det.bbox.x << ", " << det.bbox.y 
                  << ", " << det.bbox.width << ", " << det.bbox.height << ")" << std::endl;
        std::cout << "  마스크 폴리곤 점 개수: " << det.mask_polygon.size() << std::endl;
    }
    
    // 원본 Python 코드와 동일한 결과 형식으로 마스크 정보 출력
    cv::Mat image = cv::imread(image_path);
    auto mask_results = model.get_mask_results(detections, image.size());
    
    std::cout << "\n=== 마스크 결과 (Python 코드와 동일한 형식) ===" << std::endl;
    
    for (size_t i = 0; i < mask_results.xy.size(); i++) {
        std::cout << "객체 " << i + 1 << " 마스크:" << std::endl;
        
        // xy: mask in polygon format
        std::cout << "  xy 좌표 (절대값): ";
        for (const auto& point : mask_results.xy[i]) {
            std::cout << "(" << point.x << "," << point.y << ") ";
        }
        std::cout << std::endl;
        
        // xyn: normalized coordinates
        std::cout << "  xyn 좌표 (정규화): ";
        for (const auto& point : mask_results.xyn[i]) {
            std::cout << "(" << point.x << "," << point.y << ") ";
        }
        std::cout << std::endl;
        
        // masks: matrix format
        std::cout << "  마스크 매트릭스 크기: " << mask_results.masks[i].rows 
                  << "x" << mask_results.masks[i].cols << std::endl;
    }
    
    return 0;
}
