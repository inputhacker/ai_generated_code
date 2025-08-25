#include "HandDetector.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "사용법: " << argv[0] 
                  << " <palm_model_path> <landmark_model_path> <mediapipe_graph_path> <image_path>" 
                  << std::endl;
        return -1;
    }
    
    std::string palm_model_path = argv[1];
    std::string landmark_model_path = argv[2];
    std::string mediapipe_graph_path = argv[3];
    std::string image_path = argv[4];
    
    // Initialize hand detector
    HandDetector detector;
    if (!detector.initialize(palm_model_path, landmark_model_path, mediapipe_graph_path)) {
        std::cerr << "HandDetector 초기화에 실패하였습니다." << std::endl;
        return -1;
    }
    
    // Load test image
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        std::cerr << "이미지를 로드할 수 없습니다: " << image_path << std::endl;
        return -1;
    }
    
    std::cout << "손 검출을 시작합니다..." << std::endl;
    
    // Detect index finger tip
    cv::Point2f index_tip = detector.detectIndexFingerTip(image);
    
    if (index_tip.x >= 0 && index_tip.y >= 0) {
        std::cout << "검지 끝 좌표: (" << index_tip.x << ", " << index_tip.y << ")" << std::endl;
        
        // Draw result on image
        cv::circle(image, index_tip, 5, cv::Scalar(0, 255, 0), -1);
        cv::putText(image, "Index Tip", cv::Point(index_tip.x + 10, index_tip.y - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        
        // Display result
        cv::imshow("Hand Detection Result", image);
        cv::waitKey(0);
        cv::destroyAllWindows();
        
        // Save result
        cv::imwrite("result_hand_detection.jpg", image);
        std::cout << "결과가 result_hand_detection.jpg에 저장되었습니다." << std::endl;
    } else {
        std::cout << "검지 끝을 찾을 수 없습니다." << std::endl;
    }
    
    return 0;
}
