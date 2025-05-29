cmake_minimum_required(VERSION 3.10)
project(simple_agent_using_llm VERSION 1.0)

# C++17 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 헤더 파일 디렉토리
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 소스 디렉토리 변수 설정
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 소스 파일 목록
# src 디렉토리의 모든 .cpp 파일 찾기
file(GLOB SOURCES "${SOURCE_DIR}/*.cpp")

# 실행 파일 생성
add_executable(simple_agent ${SOURCES})

# 스레드 라이브러리 연결
find_package(Threads REQUIRED)
target_link_libraries(simple_agent PRIVATE Threads::Threads)

# 빌드 디렉토리 설정
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 디버그 모드 설정
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(simple_agent PRIVATE -g -O0)
else()
    target_compile_options(simple_agent PRIVATE -O2)
endif()

# 설치 규칙
install(TARGETS simple_agent DESTINATION bin)

# 빌드 방법 안내
message(STATUS "How to build:")
message(STATUS "  1. mkdir build")
message(STATUS "  2. cd build")
message(STATUS "  3. cmake ..")
message(STATUS "  4. make")
