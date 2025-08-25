#include "HandDetector.hpp"
#include <iostream>
#include <algorithm>

HandDetector::HandDetector() : initialized_(false) {
    // Initialize Hailo device
    hailo_status status = hailo_init_device(&device_, nullptr);
    if (status != HAILO_SUCCESS) {
        std::cerr << "Hailo 장치 초기화에 실패하였습니다." << std::endl;
    }
}

HandDetector::~HandDetector() {
    if (initialized_) {
        // Cleanup Hailo resources
        hailo_release_output_vstream(&output_vstream_palm_);
        hailo_release_input_vstream(&input_vstream_palm_);
        hailo_release_output_vstream(&output_vstream_landmark_);
        hailo_release_input_vstream(&input_vstream_landmark_);
        hailo_release_network_group(&network_group_palm_);
        hailo_release_network_group(&network_group_landmark_);
        hailo_release_hef(&hef_palm_);
        hailo_release_hef(&hef_landmark_);
        hailo_release_device(&device_);
    }
}

bool HandDetector::initialize(const std::string& palm_model_path, 
                            const std::string& landmark_model_path,
                            const std::string& mediapipe_graph_path) {
    
    if (!initializeHailo(palm_model_path, landmark_model_path)) {
        std::cerr << "Hailo 초기화에 실패하였습니다." << std::endl;
        return false;
    }
    
    if (!initializeMediaPipe(mediapipe_graph_path)) {
        std::cerr << "MediaPipe 초기화에 실패하였습니다." << std::endl;
        return false;
    }
    
    initialized_ = true;
    return true;
}

bool HandDetector::initializeHailo(const std::string& palm_model_path, 
                                 const std::string& landmark_model_path) {
    
    // Load palm detection model
    hailo_status status = hailo_create_hef_file(&hef_palm_, palm_model_path.c_str());
    if (status != HAILO_SUCCESS) {
        std::cerr << "Palm 모델 로드에 실패하였습니다: " << palm_model_path << std::endl;
        return false;
    }
    
    // Load hand landmark model
    status = hailo_create_hef_file(&hef_landmark_, landmark_model_path.c_str());
    if (status != HAILO_SUCCESS) {
        std::cerr << "Landmark 모델 로드에 실패하였습니다: " << landmark_model_path << std::endl;
        return false;
    }
    
    // Configure network groups
    status = hailo_configure_device(&device_, &hef_palm_, &network_group_palm_);
    if (status != HAILO_SUCCESS) {
        std::cerr << "Palm 네트워크 그룹 구성에 실패하였습니다." << std::endl;
        return false;
    }
    
    status = hailo_configure_device(&device_, &hef_landmark_, &network_group_landmark_);
    if (status != HAILO_SUCCESS) {
        std::cerr << "Landmark 네트워크 그룹 구성에 실패하였습니다." << std::endl;
        return false;
    }
    
    // Create input/output streams for palm detection
    hailo_input_vstream_params_by_name_t palm_input_params;
    hailo_output_vstream_params_by_name_t palm_output_params;
    
    status = hailo_make_input_vstream_params(&network_group_palm_, true, HAILO_FORMAT_TYPE_UINT8,
                                           &palm_input_params, nullptr);
    if (status != HAILO_SUCCESS) {
        std::cerr << "Palm 입력 스트림 파라미터 생성에 실패하였습니다." << std::endl;
        return false;
    }
    
    status = hailo_make_output_vstream_params(&network_group_palm_, true, HAILO_FORMAT_TYPE_FLOAT32,
                                            &palm_output_params, nullptr);
    if (status != HAILO_SUCCESS) {
        std::cerr << "Palm 출력 스트림 파라미터 생성에 실패하였습니다." << std::endl;
        return false;
    }
    
    // Create virtual streams for palm detection
    status = hailo_create_input_vstream(&network_group_palm_, &palm_input_params, 1, &input_vstream_palm_);
    if (status != HAILO_SUCCESS) {
        std::cerr << "Palm 입력 스트림 생성에 실패하였습니다." << std::endl;
        return false;
    }
    
    status = hailo_create_output_vstream(&network_group_palm_, &palm_output_params, 1, &output_vstream_palm_);
    if (status != HAILO_SUCCESS) {
        std::cerr << "Palm 출력 스트림 생성에 실패하였습니다." << std::endl;
        return false;
    }
    
    // Create input/output streams for hand landmarks (similar process)
    hailo_input_vstream_params_by_name_t landmark_input_params;
    hailo_output_vstream_params_by_name_t landmark_output_params;
    
    status = hailo_make_input_vstream_params(&network_group_landmark_, true, HAILO_FORMAT_TYPE_UINT8,
                                           &landmark_input_params, nullptr);
    status = hailo_make_output_vstream_params(&network_group_landmark_, true, HAILO_FORMAT_TYPE_FLOAT32,
                                            &landmark_output_params, nullptr);
    
    status = hailo_create_input_vstream(&network_group_landmark_, &landmark_input_params, 1, &input_vstream_landmark_);
    status = hailo_create_output_vstream(&network_group_landmark_, &landmark_output_params, 1, &output_vstream_landmark_);
    
    return true;
}

bool HandDetector::initializeMediaPipe(const std::string& graph_path) {
    // MediaPipe graph configuration for post-processing
    std::string calculator_graph_config_contents;
    
    // Read graph configuration file
    std::ifstream file(graph_path);
    if (!file.is_open()) {
        std::cerr << "그래프 설정 파일을 열 수 없습니다: " << graph_path << std::endl;
        return false;
    }
    
    calculator_graph_config_contents.assign((std::istreambuf_iterator<char>(file)),
                                          std::istreambuf_iterator<char>());
    
    mediapipe::CalculatorGraphConfig config;
    if (!mediapipe::ParseTextProto<mediapipe::CalculatorGraphConfig>(
            calculator_graph_config_contents, &config)) {
        std::cerr << "그래프 설정 파싱에 실패하였습니다." << std::endl;
        return false;
    }
    
    graph_ = std::make_unique<mediapipe::CalculatorGraph>();
    auto status = graph_->Initialize(config);
    if (!status.ok()) {
        std::cerr << "그래프 초기화에 실패하였습니다: " << status.ToString() << std::endl;
        return false;
    }
    
    return true;
}

cv::Point2f HandDetector::detectIndexFingerTip(const cv::Mat& image) {
    if (!initialized_) {
        std::cerr << "HandDetector가 초기화되지 않았습니다." << std::endl;
        return cv::Point2f(-1, -1);
    }
    
    // Step 1: Detect palms
    std::vector<PalmDetection> palms = detectPalms(image);
    
    if (palms.empty()) {
        std::cout << "손바닥이 검출되지 않았습니다." << std::endl;
        return cv::Point2f(-1, -1);
    }
    
    // Use the palm with highest confidence
    auto best_palm = std::max_element(palms.begin(), palms.end(),
        [](const PalmDetection& a, const PalmDetection& b) {
            return a.confidence < b.confidence;
        });
    
    // Step 2: Detect hand landmarks
    HandLandmarks landmarks = detectHandLandmarks(image, best_palm->bounding_box);
    
    if (landmarks.landmarks.empty() || landmarks.landmarks.size() <= INDEX_FINGER_TIP_ID) {
        std::cout << "손 랜드마크 검출에 실패하였습니다." << std::endl;
        return cv::Point2f(-1, -1);
    }
    
    // Step 3: Return index finger tip coordinates
    return landmarks.landmarks[INDEX_FINGER_TIP_ID];
}

std::vector<PalmDetection> HandDetector::detectPalms(const cv::Mat& image) {
    std::vector<PalmDetection> results;
    
    // Preprocess image for palm detection
    cv::Mat processed_image = preprocessImageForPalm(image);
    
    // Run inference using Hailo
    hailo_status status = hailo_vstream_write_raw_buffer(&input_vstream_palm_, 
                                                       processed_image.data, 
                                                       processed_image.total() * processed_image.elemSize());
    
    if (status != HAILO_SUCCESS) {
        std::cerr << "Palm 모델 입력 전송에 실패하였습니다." << std::endl;
        return results;
    }
    
    // Get output
    size_t output_size;
    hailo_get_output_vstream_frame_size(&output_vstream_palm_, &output_size);
    
    std::vector<float> output_data(output_size / sizeof(float));
    status = hailo_vstream_read_raw_buffer(&output_vstream_palm_, 
                                         output_data.data(), 
                                         output_size);
    
    if (status != HAILO_SUCCESS) {
        std::cerr << "Palm 모델 출력 수신에 실패하였습니다." << std::endl;
        return results;
    }
    
    // Post-process results
    results = postprocessPalmDetection(output_data, image.cols, image.rows);
    
    return results;
}

HandLandmarks HandDetector::detectHandLandmarks(const cv::Mat& image, const cv::Rect& palm_roi) {
    HandLandmarks result;
    
    // Preprocess image for landmark detection
    cv::Mat processed_image = preprocessImageForLandmark(image, palm_roi);
    
    // Run inference using Hailo
    hailo_status status = hailo_vstream_write_raw_buffer(&input_vstream_landmark_, 
                                                       processed_image.data, 
                                                       processed_image.total() * processed_image.elemSize());
    
    if (status != HAILO_SUCCESS) {
        std::cerr << "Landmark 모델 입력 전송에 실패하였습니다." << std::endl;
        return result;
    }
    
    // Get output
    size_t output_size;
    hailo_get_output_vstream_frame_size(&output_vstream_landmark_, &output_size);
    
    std::vector<float> output_data(output_size / sizeof(float));
    status = hailo_vstream_read_raw_buffer(&output_vstream_landmark_, 
                                         output_data.data(), 
                                         output_size);
    
    if (status != HAILO_SUCCESS) {
        std::cerr << "Landmark 모델 출력 수신에 실패하였습니다." << std::endl;
        return result;
    }
    
        // Post-process results
    result = postprocessHandLandmarks(output_data, palm_roi);
    
    return result;
}

cv::Mat HandDetector::preprocessImageForPalm(const cv::Mat& image) {
    cv::Mat processed;
    
    // Resize image to model input size (typically 192x192 for palm detection)
    cv::resize(image, processed, cv::Size(192, 192));
    
    // Convert BGR to RGB
    cv::cvtColor(processed, processed, cv::COLOR_BGR2RGB);
    
    // Normalize pixel values to [0, 1] range
    processed.convertTo(processed, CV_32F, 1.0/255.0);
    
    return processed;
}

cv::Mat HandDetector::preprocessImageForLandmark(const cv::Mat& image, const cv::Rect& roi) {
    cv::Mat processed;
    
    // Extract ROI from image
    cv::Mat roi_image = image(roi);
    
    // Resize to landmark model input size (typically 224x224)
    cv::resize(roi_image, processed, cv::Size(224, 224));
    
    // Convert BGR to RGB
    cv::cvtColor(processed, processed, cv::COLOR_BGR2RGB);
    
    // Normalize pixel values to [0, 1] range
    processed.convertTo(processed, CV_32F, 1.0/255.0);
    
    return processed;
}

std::vector<PalmDetection> HandDetector::postprocessPalmDetection(const std::vector<float>& output_data,
                                                                 int image_width, int image_height) {
    std::vector<PalmDetection> detections;
    
    // Assuming output format: [num_detections, 6] where 6 = [x, y, w, h, confidence, class]
    const int num_detections = output_data.size() / 6;
    const float confidence_threshold = 0.5f;
    
    for (int i = 0; i < num_detections; ++i) {
        int base_idx = i * 6;
        
        float confidence = output_data[base_idx + 4];
        if (confidence < confidence_threshold) {
            continue;
        }
        
        // Extract bounding box coordinates (normalized to [0, 1])
        float x_center = output_data[base_idx];
        float y_center = output_data[base_idx + 1];
        float width = output_data[base_idx + 2];
        float height = output_data[base_idx + 3];
        
        // Convert to pixel coordinates
        int x = static_cast<int>((x_center - width / 2.0f) * image_width);
        int y = static_cast<int>((y_center - height / 2.0f) * image_height);
        int w = static_cast<int>(width * image_width);
        int h = static_cast<int>(height * image_height);
        
        // Ensure bounding box is within image bounds
        x = std::max(0, std::min(x, image_width - 1));
        y = std::max(0, std::min(y, image_height - 1));
        w = std::min(w, image_width - x);
        h = std::min(h, image_height - y);
        
        PalmDetection detection;
        detection.bounding_box = cv::Rect(x, y, w, h);
        detection.confidence = confidence;
        
        detections.push_back(detection);
    }
    
    return detections;
}

HandLandmarks HandDetector::postprocessHandLandmarks(const std::vector<float>& output_data,
                                                   const cv::Rect& roi) {
    HandLandmarks result;
    
    // MediaPipe hand landmarks: 21 points, each with (x, y, z) coordinates
    const int num_landmarks = 21;
    const int coords_per_landmark = 3;
    
    if (output_data.size() < num_landmarks * coords_per_landmark) {
        std::cerr << "출력 데이터 크기가 예상보다 작습니다." << std::endl;
        return result;
    }
    
    result.landmarks.reserve(num_landmarks);
    
    for (int i = 0; i < num_landmarks; ++i) {
        int base_idx = i * coords_per_landmark;
        
        // Extract normalized coordinates
        float x_norm = output_data[base_idx];
        float y_norm = output_data[base_idx + 1];
        // z coordinate is not used for 2D point detection
        
        // Convert to pixel coordinates relative to ROI
        float x_pixel = x_norm * roi.width + roi.x;
        float y_pixel = y_norm * roi.height + roi.y;
        
        result.landmarks.emplace_back(x_pixel, y_pixel);
    }
    
    result.confidence = 1.0f; // Assuming high confidence if landmarks are detected
    
    return result;
}
