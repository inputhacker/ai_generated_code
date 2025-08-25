# 빌드 디렉토리 생성
mkdir build && cd build

# CMake 구성 (경로를 실제 환경에 맞게 수정)
cmake .. -DMEDIAPIPE_ROOT_PATH=/path/to/mediapipe

# 빌드
make -j$(nproc)

# 실행
./HandDetector palm_model.hef landmark_model.hef hand_detection_graph.pbtxt test_image.jpg
