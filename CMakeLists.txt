cmake_minimum_required(VERSION 3.10)
project(UserAgent VERSION 1.0)

# C++17 표준 설정
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 헤더 파일 디렉토리
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 소스 파일 목록
set(SOURCES
    Message.cpp
    VectorDB.cpp
    LLMAgent.cpp
    ShortTermMemory.cpp
    LongTermMemory.cpp
    MemoryManager.cpp
    UserAgent.cpp
    main.cpp
)

# 실행 파일 생성
add_executable(user_agent ${SOURCES})

# 스레드 라이브러리 연결 (필요한 경우)
find_package(Threads REQUIRED)
target_link_libraries(user_agent PRIVATE Threads::Threads)

# 빌드 디렉토리 설정
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 디버그 모드 설정 (선택 사항)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(user_agent PRIVATE -g -O0)
else()
    target_compile_options(user_agent PRIVATE -O2)
endif()

# 설치 규칙 (선택 사항)
install(TARGETS user_agent DESTINATION bin)

# 빌드 방법 안내
message(STATUS "빌드 방법:")
message(STATUS "  1. mkdir build")
message(STATUS "  2. cd build")
message(STATUS "  3. cmake ..")
message(STATUS "  4. make")
