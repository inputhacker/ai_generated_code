#ifndef HAILO_YOLO11_SEG_HPP
#define HAILO_YOLO11_SEG_HPP

#include <iostream>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>
#include "hailo/hailort.h"
#include "hailo/hailort.hpp"

using namespace hailort;

struct MaskResult {
    std::vector<std::vector<cv::Point2f>> xy;     // mask in polygon format
    std::vector<std::vector<cv::Point2f>> xyn;    // normalized polygon
    std::vector<cv::Mat> masks;                   // mask in matrix format
};

struct DetectionResult {
    cv::Rect2f bbox;
    float confidence;
    int class_id;
    std::vector<cv::Point2f> mask_polygon;
    cv::Mat mask_matrix;
};

class HailoYOLO11Seg {
private:
    std::string hef_path_;
    std::unique_ptr<Device> device_;
    std::shared_ptr<ConfiguredNetworkGroup> network_group_;
    std::unique_ptr<InputVStream> input_vstream_;
    std::vector<std::unique_ptr<OutputVStream>> output_vstreams_;
    
    hailo_3d_image_shape_t input_shape_;
    std::map<std::string, hailo_3d_image_shape_t> output_shapes_;
    
    float conf_threshold_ = 0.25f;
    float iou_threshold_ = 0.45f;
    int num_classes_ = 80;

public:
    HailoYOLO11Seg(const std::string& hef_path);
    ~HailoYOLO11Seg();
    
    hailo_status initialize();
    std::vector<DetectionResult> predict(const std::string& image_path);
    MaskResult get_mask_results(const std::vector<DetectionResult>& detections, 
                               const cv::Size& original_size);
    
private:
    cv::Mat preprocess_image(const std::string& image_path, cv::Size& original_size);
    hailo_status run_inference(const cv::Mat& input_image, 
                              std::map<std::string, std::vector<uint8_t>>& outputs);
    std::vector<DetectionResult> postprocess(
        const std::map<std::string, std::vector<uint8_t>>& outputs,
        const cv::Size& original_size);
    
    std::vector<DetectionResult> apply_nms(const std::vector<DetectionResult>& detections);
    void decode_masks(std::vector<DetectionResult>& detections,
                     const std::vector<uint8_t>& mask_output,
                     const cv::Size& original_size);
    std::vector<cv::Point2f> mask_to_polygon(const cv::Mat& mask);
};

#endif // HAILO_YOLO11_SEG_HPP
