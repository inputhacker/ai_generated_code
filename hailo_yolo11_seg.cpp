#include "hailo_yolo11_seg.hpp"
#include <algorithm>
#include <numeric>

HailoYOLO11Seg::HailoYOLO11Seg(const std::string& hef_path) 
    : hef_path_(hef_path) {}

HailoYOLO11Seg::~HailoYOLO11Seg() {
    if (input_vstream_) {
        input_vstream_->abort();
    }
    for (auto& output_vstream : output_vstreams_) {
        if (output_vstream) {
            output_vstream->abort();
        }
    }
}

hailo_status HailoYOLO11Seg::initialize() {
    // HEF 파일 생성
    auto hef_exp = Hef::create(hef_path_);
    if (!hef_exp) {
        std::cerr << "HEF 파일 로드 실패: " << hef_exp.status() << std::endl;
        return hef_exp.status();
    }
    auto hef = hef_exp.release();

    // Device 생성
    auto device_exp = Device::create();
    if (!device_exp) {
        std::cerr << "Device 생성 실패: " << device_exp.status() << std::endl;
        return device_exp.status();
    }
    device_ = std::make_unique<Device>(device_exp.release());

    // Network Group 설정
    auto configure_params = hef.create_configure_params(HAILO_STREAM_INTERFACE_PCIE);
    if (!configure_params) {
        std::cerr << "Configure params 생성 실패: " << configure_params.status() << std::endl;
        return configure_params.status();
    }

    auto network_groups = device_->configure(hef, configure_params.value());
    if (!network_groups) {
        std::cerr << "Network group 설정 실패: " << network_groups.status() << std::endl;
        return network_groups.status();
    }
    network_group_ = std::move(network_groups->at(0));

    // Input VStream 생성
    auto input_vstream_params = network_group_->make_input_vstream_params(false, HAILO_FORMAT_TYPE_AUTO);
    if (!input_vstream_params) {
        std::cerr << "Input VStream params 생성 실패: " << input_vstream_params.status() << std::endl;
        return input_vstream_params.status();
    }

    auto input_vstreams = VStreamsBuilder::create_input_vstreams(*network_group_, input_vstream_params.value());
    if (!input_vstreams || input_vstreams->empty()) {
        std::cerr << "Input VStreams 생성 실패" << std::endl;
        return HAILO_INTERNAL_FAILURE;
    }
    input_vstream_ = std::make_unique<InputVStream>(std::move(input_vstreams->at(0)));

    // Output VStreams 생성
    auto output_vstream_params = network_group_->make_output_vstream_params(false, HAILO_FORMAT_TYPE_AUTO);
    if (!output_vstream_params) {
        std::cerr << "Output VStream params 생성 실패: " << output_vstream_params.status() << std::endl;
        return output_vstream_params.status();
    }

    auto output_vstreams = VStreamsBuilder::create_output_vstreams(*network_group_, output_vstream_params.value());
    if (!output_vstreams) {
        std::cerr << "Output VStreams 생성 실패: " << output_vstreams.status() << std::endl;
        return output_vstreams.status();
    }

    for (auto& output_vstream : output_vstreams.value()) {
        output_vstreams_.push_back(std::make_unique<OutputVStream>(std::move(output_vstream)));
    }

    // Input/Output shape 정보 저장
    input_shape_ = input_vstream_->get_info().shape;
    for (auto& output_vstream : output_vstreams_) {
        auto info = output_vstream->get_info();
        output_shapes_[info.name] = info.shape;
    }

    std::cout << "모델 초기화 완료" << std::endl;
    std::cout << "Input shape: " << input_shape_.width << "x" << input_shape_.height << "x" << input_shape_.features << std::endl;
    
    for (const auto& [name, shape] : output_shapes_) {
        std::cout << "Output " << name << " shape: " << shape.width << "x" << shape.height << "x" << shape.features << std::endl;
    }

    return HAILO_SUCCESS;
}

cv::Mat HailoYOLO11Seg::preprocess_image(const std::string& image_path, cv::Size& original_size) {
    // 이미지 로드
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        std::cerr << "이미지 로드 실패: " << image_path << std::endl;
        return cv::Mat();
    }
    
    original_size = image.size();
    
    // 입력 크기로 리사이즈
    cv::Mat resized_image;
    cv::resize(image, resized_image, cv::Size(input_shape_.width, input_shape_.height));
    
    // BGR to RGB 변환
    cv::cvtColor(resized_image, resized_image, cv::COLOR_BGR2RGB);
    
    // 정규화 (0-255 -> 0-1)
    resized_image.convertTo(resized_image, CV_32F, 1.0/255.0);
    
    return resized_image;
}

hailo_status HailoYOLO11Seg::run_inference(const cv::Mat& input_image,
                                          std::map<std::string, std::vector<uint8_t>>& outputs) {
    // Input 데이터 준비
    size_t input_frame_size = input_vstream_->get_frame_size();
    std::vector<uint8_t> input_data(input_frame_size);
    
    // OpenCV Mat을 HailoRT 형식으로 변환
    cv::Mat input_hwc;
    if (input_image.channels() == 3) {
        // RGB 이미지를 HWC 형식으로 변환
        std::memcpy(input_data.data(), input_image.data, input_frame_size);
    }
    
    // Input 전송
    auto status = input_vstream_->write(MemoryView(input_data.data(), input_data.size()));
    if (HAILO_SUCCESS != status) {
        std::cerr << "Input 전송 실패: " << status << std::endl;
        return status;
    }
    
    // Output 수신
    for (auto& output_vstream : output_vstreams_) {
        size_t output_frame_size = output_vstream->get_frame_size();
        std::vector<uint8_t> output_data(output_frame_size);
        
        status = output_vstream->read(MemoryView(output_data.data(), output_data.size()));
        if (HAILO_SUCCESS != status) {
            std::cerr << "Output 수신 실패: " << status << std::endl;
            return status;
        }
        
        outputs[output_vstream->get_info().name] = std::move(output_data);
    }
    
    return HAILO_SUCCESS;
}

std::vector<DetectionResult> HailoYOLO11Seg::postprocess(
    const std::map<std::string, std::vector<uint8_t>>& outputs,
    const cv::Size& original_size) {
    
    std::vector<DetectionResult> detections;
    
    // Detection output과 mask output 분리
    std::vector<uint8_t> detection_output;
    std::vector<uint8_t> mask_output;
    
    for (const auto& [name, data] : outputs) {
        if (name.find("detect") != std::string::npos || name.find("output0") != std::string::npos) {
            detection_output = data;
        } else if (name.find("proto") != std::string::npos || name.find("output1") != std::string::npos) {
            mask_output = data;
        }
    }
    
    if (detection_output.empty()) {
        std::cerr << "Detection output을 찾을 수 없습니다." << std::endl;
        return detections;
    }
    
    // Detection 결과 파싱 (quantized format에서 float로 변환)
    const float* det_data = reinterpret_cast<const float*>(detection_output.data());
    int num_detections = detection_output.size() / ((4 + 1 + num_classes_ + 32) * sizeof(float)); // +32 for mask coefficients
    
    for (int i = 0; i < num_detections; i++) {
        int offset = i * (4 + 1 + num_classes_ + 32);
        
        // Bounding box (center_x, center_y, width, height)
        float cx = det_data[offset] * input_shape_.width;
        float cy = det_data[offset + 1] * input_shape_.height;
        float w = det_data[offset + 2] * input_shape_.width;
        float h = det_data[offset + 3] * input_shape_.height;
        
        // Confidence
        float obj_conf = det_data[offset + 4];
        
        // 클래스별 확률
        float max_class_conf = 0.0f;
        int max_class_id = -1;
        
        for (int c = 0; c < num_classes_; c++) {
            float class_conf = det_data[offset + 5 + c];
            if (class_conf > max_class_conf) {
                max_class_conf = class_conf;
                max_class_id = c;
            }
        }
        
        float confidence = obj_conf * max_class_conf;
        
        if (confidence > conf_threshold_) {
            DetectionResult det;
            
            // 바운딩 박스를 원본 이미지 좌표로 변환
            float scale_x = static_cast<float>(original_size.width) / input_shape_.width;
            float scale_y = static_cast<float>(original_size.height) / input_shape_.height;
            
            float x1 = (cx - w / 2.0f) * scale_x;
            float y1 = (cy - h / 2.0f) * scale_y;
            float x2 = (cx + w / 2.0f) * scale_x;
            float y2 = (cy + h / 2.0f) * scale_y;
            
            det.bbox = cv::Rect2f(x1, y1, x2 - x1, y2 - y1);
            det.confidence = confidence;
            det.class_id = max_class_id;
            
            detections.push_back(det);
        }
    }
    
    // NMS 적용
    detections = apply_nms(detections);
    
    // 마스크 디코딩
    if (!mask_output.empty()) {
        decode_masks(detections, mask_output, original_size);
    }
    
    return detections;
}

std::vector<DetectionResult> HailoYOLO11Seg::apply_nms(const std::vector<DetectionResult>& detections) {
    std::vector<cv::Rect2f> boxes;
    std::vector<float> scores;
    std::vector<int> indices;
    
    for (const auto& det : detections) {
        boxes.push_back(det.bbox);
        scores.push_back(det.confidence);
    }
    
    cv::dnn::NMSBoxes(boxes, scores, conf_threshold_, iou_threshold_, indices);
    
    std::vector<DetectionResult> nms_detections;
    for (int idx : indices) {
        nms_detections.push_back(detections[idx]);
    }
    
    return nms_detections;
}

void HailoYOLO11Seg::decode_masks(std::vector<DetectionResult>& detections,
                                 const std::vector<uint8_t>& mask_output,
                                 const cv::Size& original_size) {
    if (mask_output.empty()) return;
    
    // 마스크 프로토타입 디코딩 (일반적으로 160x160x32 형태)
    const float* mask_data = reinterpret_cast<const float*>(mask_output.data());
    int mask_h = input_shape_.height / 4;  // 일반적으로 1/4 크기
    int mask_w = input_shape_.width / 4;
    int mask_dim = 32;  // 프로토타입 차원
    
    // 프로토타입 마스크 생성
    cv::Mat prototype_masks(mask_h * mask_w, mask_dim, CV_32F, (void*)mask_data);
    
    for (auto& detection : detections) {
        // 각 detection에 대한 마스크 계수 (detection output의 마지막 32개 값)
        // 실제 구현에서는 detection output에서 마스크 계수를 추출해야 함
        cv::Mat mask_coeffs = cv::Mat::zeros(1, mask_dim, CV_32F);
        // mask_coeffs에 실제 계수 값들을 채워넣어야 함
        
        // 마스크 생성 (prototype_masks * mask_coefficients)
        cv::Mat mask_pred = mask_coeffs * prototype_masks.t();
        mask_pred = mask_pred.reshape(0, mask_h);
        
        // Sigmoid 활성화
        cv::exp(-mask_pred, mask_pred);
        mask_pred = 1.0 / (1.0 + mask_pred);
        
        // 원본 이미지 크기로 리사이즈
        cv::Mat resized_mask;
        cv::resize(mask_pred, resized_mask, original_size);
        
        // 바운딩 박스 영역 마스킹
        cv::Rect roi = detection.bbox & cv::Rect(0, 0, original_size.width, original_size.height);
        if (roi.area() > 0) {
            cv::Mat bbox_mask = resized_mask(roi).clone();
            cv::threshold(bbox_mask, bbox_mask, 0.5, 1.0, cv::THRESH_BINARY);
            
            detection.mask_matrix = bbox_mask;
            detection.mask_polygon = mask_to_polygon(bbox_mask);
        }
    }
}

std::vector<cv::Point2f> HailoYOLO11Seg::mask_to_polygon(const cv::Mat& mask) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    std::vector<cv::Point2f> polygon;
    if (!contours.empty()) {
        // 가장 큰 contour 선택
        auto largest_contour = *std::max_element(contours.begin(), contours.end(),
            [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
                return cv::contourArea(a) < cv::contourArea(b);
            });
        
        // 다각형 근사
        std::vector<cv::Point> approx_contour;
        double epsilon = 0.02 * cv::arcLength(largest_contour, true);
        cv::approxPolyDP(largest_contour, approx_contour, epsilon, true);
        
        for (const auto& point : approx_contour) {
            polygon.emplace_back(static_cast<float>(point.x), static_cast<float>(point.y));
        }
    }
    
    return polygon;
}

std::vector<DetectionResult> HailoYOLO11Seg::predict(const std::string& image_path) {
    cv::Size original_size;
    cv::Mat preprocessed = preprocess_image(image_path, original_size);
    
    if (preprocessed.empty()) {
        return {};
    }
    
    std::map<std::string, std::vector<uint8_t>> outputs;
    auto status = run_inference(preprocessed, outputs);
    
    if (status != HAILO_SUCCESS) {
        std::cerr << "추론 실행 실패: " << status << std::endl;
        return {};
    }
    
    return postprocess(outputs, original_size);
}

MaskResult HailoYOLO11Seg::get_mask_results(const std::vector<DetectionResult>& detections,
                                           const cv::Size& original_size) {
    MaskResult result;
    
    for (const auto& det : detections) {
        // xy: mask in polygon format (absolute coordinates)
        result.xy.push_back(det.mask_polygon);
        
        // xyn: normalized polygon coordinates
        std::vector<cv::Point2f> normalized_polygon;
        for (const auto& point : det.mask_polygon) {
            normalized_polygon.emplace_back(
                point.x / original_size.width,
                point.y / original_size.height
            );
        }
        result.xyn.push_back(normalized_polygon);
        
        // masks: mask in matrix format
        result.masks.push_back(det.mask_matrix);
    }
    
    return result;
}
