#ifndef HAND_DETECTOR_HPP
#define HAND_DETECTOR_HPP

#include <opencv2/opencv.hpp>
#include <mediapipe/framework/calculator_framework.h>
#include <mediapipe/framework/formats/image_frame.h>
#include <mediapipe/framework/formats/image_frame_opencv.h>
#include <mediapipe/framework/port/opencv_imgproc_inc.h>
#include <mediapipe/framework/port/parse_text_proto.h>
#include <mediapipe/framework/formats/landmark.pb.h>
#include <hailo/hailort.h>
#include <vector>
#include <memory>

struct PalmDetection {
    cv::Rect bounding_box;
    float confidence;
    std::vector<cv::Point2f> keypoints;
};

struct HandLandmarks {
    std::vector<cv::Point2f> landmarks;
    float confidence;
};

class HandDetector {
public:
    HandDetector();
    ~HandDetector();

    bool initialize(const std::string& palm_model_path, 
                   const std::string& landmark_model_path,
                   const std::string& mediapipe_graph_path);
    
    cv::Point2f detectIndexFingerTip(const cv::Mat& image);
    
    std::vector<PalmDetection> detectPalms(const cv::Mat& image);
    HandLandmarks detectHandLandmarks(const cv::Mat& image, const cv::Rect& palm_roi);

private:
    // MediaPipe components
    std::unique_ptr<mediapipe::CalculatorGraph> graph_;
    
    // Hailo components
    hailo_device device_;
    hailo_hef hef_palm_;
    hailo_hef hef_landmark_;
    hailo_configured_network_group network_group_palm_;
    hailo_configured_network_group network_group_landmark_;
    hailo_input_vstream input_vstream_palm_;
    hailo_output_vstream output_vstream_palm_;
    hailo_input_vstream input_vstream_landmark_;
    hailo_output_vstream output_vstream_landmark_;
    
    bool initialized_;
    
    // Helper functions
    bool initializeHailo(const std::string& palm_model_path, 
                        const std::string& landmark_model_path);
    bool initializeMediaPipe(const std::string& graph_path);
    
    cv::Mat preprocessImageForPalm(const cv::Mat& image);
    cv::Mat preprocessImageForLandmark(const cv::Mat& image, const cv::Rect& roi);
    
    std::vector<PalmDetection> postprocessPalmDetection(const std::vector<float>& output_data,
                                                       int image_width, int image_height);
    HandLandmarks postprocessHandLandmarks(const std::vector<float>& output_data,
                                          const cv::Rect& roi);
    
    static constexpr int INDEX_FINGER_TIP_ID = 8; // MediaPipe hand landmark index for index finger tip
};

#endif // HAND_DETECTOR_HPP
