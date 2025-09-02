#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <hailo/hailort.hpp>

class DINOv2HailoInference {
private:
    std::unique_ptr<hailort::Device> device_;
    std::shared_ptr<hailort::ConfiguredNetworkGroup> network_group_;
    std::shared_ptr<hailort::InputVStream> input_vstream_;
    std::shared_ptr<hailort::OutputVStream> output_vstream_;
    
    size_t input_frame_size_;
    size_t output_frame_size_;
    hailo_format_t input_format_;
    hailo_format_t output_format_;
    
    // DINOv2-small 모델의 일반적인 입력 크기
    static constexpr int INPUT_HEIGHT = 224;
    static constexpr int INPUT_WIDTH = 224;
    static constexpr int INPUT_CHANNELS = 3;

public:
    DINOv2HailoInference() = default;
    ~DINOv2HailoInference() = default;

    bool initialize(const std::string& hef_path) {
        try {
            // 1. Hailo 디바이스 생성
            auto device_exp = hailort::Device::create();
            if (!device_exp) {
                std::cerr << "Failed to create device: " << device_exp.status() << std::endl;
                return false;
            }
            device_ = device_exp.release();

            // 2. HEF 파일 로드
            auto hef_exp = hailort::Hef::create(hef_path);
            if (!hef_exp) {
                std::cerr << "Failed to create HEF: " << hef_exp.status() << std::endl;
                return false;
            }
            auto hef = hef_exp.release();

            // 3. Network Group 설정
            auto configure_params = hef.create_configure_params(HAILO_STREAM_INTERFACE_PCIE);
            if (!configure_params) {
                std::cerr << "Failed to create configure params: " << configure_params.status() << std::endl;
                return false;
            }

            auto network_groups = device_->configure(hef, configure_params.value());
            if (!network_groups) {
                std::cerr << "Failed to configure device: " << network_groups.status() << std::endl;
                return false;
            }

            if (network_groups->size() != 1) {
                std::cerr << "Expected single network group, got " << network_groups->size() << std::endl;
                return false;
            }

            network_group_ = std::make_shared<hailort::ConfiguredNetworkGroup>(std::move(network_groups->at(0)));

            // 4. Input/Output VStreams 생성
            auto input_vstream_params = network_group_->make_input_vstream_params(
                true, HAILO_FORMAT_TYPE_FLOAT32, HAILO_DEFAULT_VSTREAM_TIMEOUT_MS, HAILO_DEFAULT_VSTREAM_QUEUE_SIZE
            );
            if (!input_vstream_params) {
                std::cerr << "Failed to create input vstream params: " << input_vstream_params.status() << std::endl;
                return false;
            }

            auto output_vstream_params = network_group_->make_output_vstream_params(
                true, HAILO_FORMAT_TYPE_FLOAT32, HAILO_DEFAULT_VSTREAM_TIMEOUT_MS, HAILO_DEFAULT_VSTREAM_QUEUE_SIZE
            );
            if (!output_vstream_params) {
                std::cerr << "Failed to create output vstream params: " << output_vstream_params.status() << std::endl;
                return false;
            }

            auto input_vstreams = hailort::VStreamsBuilder::create_input_vstreams(*network_group_, input_vstream_params.value());
            if (!input_vstreams) {
                std::cerr << "Failed to create input vstreams: " << input_vstreams.status() << std::endl;
                return false;
            }

            auto output_vstreams = hailort::VStreamsBuilder::create_output_vstreams(*network_group_, output_vstream_params.value());
            if (!output_vstreams) {
                std::cerr << "Failed to create output vstreams: " << output_vstreams.status() << std::endl;
                return false;
            }

            if (input_vstreams->size() != 1 || output_vstreams->size() != 1) {
                std::cerr << "Expected single input and output stream" << std::endl;
                return false;
            }

            input_vstream_ = std::make_shared<hailort::InputVStream>(std::move(input_vstreams->at(0)));
            output_vstream_ = std::make_shared<hailort::OutputVStream>(std::move(output_vstreams->at(0)));

            // 5. Stream 정보 가져오기
            auto input_info = input_vstream_->get_info();
            auto output_info = output_vstream_->get_info();

            input_format_ = input_info.format;
            output_format_ = output_info.format;
            input_frame_size_ = hailort::get_frame_size(input_info, input_format_);
            output_frame_size_ = hailort::get_frame_size(output_info, output_format_);

            std::cout << "Hailo DINOv2 모델이 성공적으로 초기화되었습니다." << std::endl;
            std::cout << "Input frame size: " << input_frame_size_ << " bytes" << std::endl;
            std::cout << "Output frame size: " << output_frame_size_ << " bytes" << std::endl;

            return true;

        } catch (const std::exception& e) {
            std::cerr << "Exception during initialization: " << e.what() << std::endl;
            return false;
        }
    }

    cv::Mat preprocessImage(const cv::Mat& input_image) {
        cv::Mat processed_image;
        
        // 1. BGR to RGB 변환
        cv::cvtColor(input_image, processed_image, cv::COLOR_BGR2RGB);
        
        // 2. 크기 조정
        cv::resize(processed_image, processed_image, cv::Size(INPUT_WIDTH, INPUT_HEIGHT));
        
        // 3. 정규화 (ImageNet 표준)
        processed_image.convertTo(processed_image, CV_32F, 1.0/255.0);
        
        // ImageNet mean과 std를 사용한 정규화
        cv::Scalar mean(0.485, 0.456, 0.406);
        cv::Scalar std(0.229, 0.224, 0.225);
        
        std::vector<cv::Mat> channels(3);
        cv::split(processed_image, channels);
        
        for (int i = 0; i < 3; ++i) {
            channels[i] = (channels[i] - mean[i]) / std[i];
        }
        
        cv::merge(channels, processed_image);
        
        return processed_image;
    }

    std::vector<float> extractEmbedding(const cv::Mat& image) {
        try {
            // 1. 이미지 전처리
            cv::Mat preprocessed = preprocessImage(image);
            
            // 2. 입력 데이터 준비 (HWC to CHW 변환)
            std::vector<float> input_data(INPUT_HEIGHT * INPUT_WIDTH * INPUT_CHANNELS);
            
            for (int c = 0; c < INPUT_CHANNELS; ++c) {
                for (int h = 0; h < INPUT_HEIGHT; ++h) {
                    for (int w = 0; w < INPUT_WIDTH; ++w) {
                        int dst_idx = c * INPUT_HEIGHT * INPUT_WIDTH + h * INPUT_WIDTH + w;
                        input_data[dst_idx] = preprocessed.at<cv::Vec3f>(h, w)[c];
                    }
                }
            }
            
            // 3. 추론 실행
            auto status = input_vstream_->write(MemoryView(input_data.data(), input_frame_size_));
            if (HAILO_SUCCESS != status) {
                std::cerr << "Failed to write to input stream: " << status << std::endl;
                return {};
            }
            
            // 4. 결과 읽기
            std::vector<uint8_t> output_buffer(output_frame_size_);
            status = output_vstream_->read(MemoryView(output_buffer.data(), output_buffer.size()));
            if (HAILO_SUCCESS != status) {
                std::cerr << "Failed to read from output stream: " << status << std::endl;
                return {};
            }
            
            // 5. 출력 데이터를 float 벡터로 변환
            std::vector<float> embedding(output_frame_size_ / sizeof(float));
            std::memcpy(embedding.data(), output_buffer.data(), output_frame_size_);
            
            return embedding;
            
        } catch (const std::exception& e) {
            std::cerr << "Exception during inference: " << e.what() << std::endl;
            return {};
        }
    }

    bool start() {
        auto status = network_group_->activate();
        if (HAILO_SUCCESS != status) {
            std::cerr << "Failed to activate network group: " << status << std::endl;
            return false;
        }
        return true;
    }

    void stop() {
        if (network_group_) {
            network_group_->shutdown();
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "사용법: " << argv[0] << " <hef_file_path> <image_file_path>" << std::endl;
        return -1;
    }

    std::string hef_path = argv[1];
    std::string image_path = argv[2];

    // 1. DINOv2 Hailo 추론 객체 생성 및 초기화
    DINOv2HailoInference dinov2_inference;
    
    if (!dinov2_inference.initialize(hef_path)) {
        std::cerr << "DINOv2 모델 초기화에 실패했습니다." << std::endl;
        return -1;
    }

    // 2. 네트워크 활성화
    if (!dinov2_inference.start()) {
        std::cerr << "네트워크 활성화에 실패했습니다." << std::endl;
        return -1;
    }

    // 3. 이미지 로드
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        std::cerr << "이미지를 로드할 수 없습니다: " << image_path << std::endl;
        dinov2_inference.stop();
        return -1;
    }

    std::cout << "이미지 로드 완료: " << image.cols << "x" << image.rows << std::endl;

    // 4. 임베딩 추출
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::vector<float> embedding = dinov2_inference.extractEmbedding(image);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    if (embedding.empty()) {
        std::cerr << "임베딩 추출에 실패했습니다." << std::endl;
        dinov2_inference.stop();
        return -1;
    }

    // 5. 결과 출력
    std::cout << "\n=== DINOv2 임베딩 결과 ===" << std::endl;
    std::cout << "임베딩 차원: " << embedding.size() << std::endl;
    std::cout << "추론 시간: " << duration.count() << "ms" << std::endl;
    
    // 임베딩 벡터의 일부 값 출력 (처음 10개 값)
    std::cout << "임베딩 샘플 값: ";
    for (size_t i = 0; i < std::min(static_cast<size_t>(10), embedding.size()); ++i) {
        std::cout << embedding[i] << " ";
    }
    std::cout << "..." << std::endl;

    // L2 정규화된 임베딩 계산
    float norm = 0.0f;
    for (float val : embedding) {
        norm += val * val;
    }
    norm = std::sqrt(norm);
    
    std::cout << "임베딩 L2 norm: " << norm << std::endl;

    // 6. 정리
    dinov2_inference.stop();
    std::cout << "프로그램이 성공적으로 완료되었습니다." << std::endl;

    return 0;
}
