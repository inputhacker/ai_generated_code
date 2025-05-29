cmake_minimum_required(VERSION 3.10)
project(ipc_via_named_pipe VERSION 1.0)

# C++17 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 헤더 파일 디렉토리
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 실행 파일 생성
add_executable(writer writer_named_pipe.cpp)
add_executable(reader reader_named_pipe.cpp)

# 빌드 디렉토리 설정
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 설치 규칙 (선택 사항)
install(TARGETS writer DESTINATION bin)
install(TARGETS reader DESTINATION bin)

# 빌드 방법 안내
message(STATUS "How to build:")
message(STATUS "  1. mkdir build")
message(STATUS "  2. cd build")
message(STATUS "  3. cmake ..")
message(STATUS "  4. make")
