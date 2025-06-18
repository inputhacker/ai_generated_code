#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

// 기본 벤더 에이전트 인터페이스 정의
class VendorAgentImpl {
public:
    virtual ~VendorAgentImpl() = default;
    virtual void initialize() = 0;
    virtual std::string processRequest(const std::string& request) = 0;
    virtual void shutdown() = 0;
};

// A 벤더의 구체적인 에이전트 구현
class AvendorAgentImpl : public VendorAgentImpl {
public:
    void initialize() override {
        std::cout << "A벤더 에이전트 초기화 중..." << std::endl;
    }
    
    std::string processRequest(const std::string& request) override {
        std::cout << "A벤더 방식으로 요청 처리: " << request << std::endl;
        return "A벤더 결과: " + request + " 처리됨";
    }
    
    void shutdown() override {
        std::cout << "A벤더 에이전트 종료 중..." << std::endl;
    }
};

// B 벤더의 구체적인 에이전트 구현
class BvendorAgentImpl : public VendorAgentImpl {
public:
    void initialize() override {
        std::cout << "B벤더 에이전트 초기화 중..." << std::endl;
    }
    
    std::string processRequest(const std::string& request) override {
        std::cout << "B벤더 방식으로 요청 처리: " << request << std::endl;
        return "B벤더 결과: " + request + " 분석 완료";
    }
    
    void shutdown() override {
        std::cout << "B벤더 에이전트 종료 중..." << std::endl;
    }
};

// C 벤더의 구체적인 에이전트 구현
class CvendorAgentImpl : public VendorAgentImpl {
public:
    void initialize() override {
        std::cout << "C벤더 에이전트 초기화 중..." << std::endl;
    }
    
    std::string processRequest(const std::string& request) override {
        std::cout << "C벤더 방식으로 요청 처리: " << request << std::endl;
        return "C벤더 결과: " + request + " 실행됨";
    }
    
    void shutdown() override {
        std::cout << "C벤더 에이전트 종료 중..." << std::endl;
    }
};

// Agent 클래스 - 클라이언트 인터페이스와 구현체 사이의 브릿지 역할
class Agent {
private:
    std::shared_ptr<VendorAgentImpl> impl;
    std::string vendorType;
    
    // 벤더 유형에 따른 구현체 생성 (팩토리 메소드)
    std::shared_ptr<VendorAgentImpl> createVendorImpl(const std::string& type) {
        if (type == "A") {
            return std::make_shared<AvendorAgentImpl>();
        } else if (type == "B") {
            return std::make_shared<BvendorAgentImpl>();
        } else if (type == "C") {
            return std::make_shared<CvendorAgentImpl>();
        } else {
            throw std::invalid_argument("지원하지 않는 벤더 유형: " + type);
        }
    }

public:
    // 생성자 - 벤더 유형을 기반으로 구현체 생성
    Agent(const std::string& type) : vendorType(type) {
        impl = createVendorImpl(type);
        impl->initialize();
    }
    
    // 소멸자
    ~Agent() {
        if (impl) {
            impl->shutdown();
        }
    }
    
    // 복사 생성자 삭제
    Agent(const Agent&) = delete;
    
    // 대입 연산자 삭제
    Agent& operator=(const Agent&) = delete;
    
    // 이동 생성자
    Agent(Agent&& other) noexcept : impl(std::move(other.impl)), vendorType(std::move(other.vendorType)) {
        other.impl = nullptr;
    }
    
    // 이동 대입 연산자
    Agent& operator=(Agent&& other) noexcept {
        if (this != &other) {
            if (impl) {
                impl->shutdown();
            }
            impl = std::move(other.impl);
            vendorType = std::move(other.vendorType);
            other.impl = nullptr;
        }
        return *this;
    }
    
    // 요청 처리 메소드 - 구현체에 위임
    std::string processRequest(const std::string& request) {
        if (!impl) {
            throw std::runtime_error("에이전트가 초기화되지 않았습니다");
        }
        return impl->processRequest(request);
    }
    
    // 벤더 유형 조회
    std::string getVendorType() const {
        return vendorType;
    }
    
    // 에이전트 재설정 메소드
    void resetWithVendor(const std::string& type) {
        if (impl) {
            impl->shutdown();
        }
        vendorType = type;
        impl = createVendorImpl(type);
        impl->initialize();
    }
};

// 사용 예시
int main() {
    try {
        // A 벤더 에이전트 생성 및 사용
        std::shared_ptr<Agent> agentA = std::make_shared<Agent>("A");
        std::string resultA = agentA->processRequest("데이터 분석 요청");
        std::cout << "처리 결과: " << resultA << std::endl;
        std::cout << "현재 벤더 유형: " << agentA->getVendorType() << std::endl;
        
        std::cout << "\n------------------------------\n" << std::endl;
        
        // B 벤더 에이전트 생성 및 사용
        std::shared_ptr<Agent> agentB = std::make_shared<Agent>("B");
        std::string resultB = agentB->processRequest("이미지 처리 요청");
        std::cout << "처리 결과: " << resultB << std::endl;
        std::cout << "현재 벤더 유형: " << agentB->getVendorType() << std::endl;
        
        std::cout << "\n------------------------------\n" << std::endl;
        
        // C 벤더 에이전트 생성 및 사용
        std::shared_ptr<Agent> agentC = std::make_shared<Agent>("C");
        std::string resultC = agentC->processRequest("텍스트 생성 요청");
        std::cout << "처리 결과: " << resultC << std::endl;
        std::cout << "현재 벤더 유형: " << agentC->getVendorType() << std::endl;
        
        std::cout << "\n------------------------------\n" << std::endl;
        
        // 에이전트 벤더 전환 예시
        std::cout << "에이전트 벤더 전환 (A → C)" << std::endl;
        agentA->resetWithVendor("C");
        std::string resultChanged = agentA->processRequest("새로운 요청");
        std::cout << "처리 결과: " << resultChanged << std::endl;
        std::cout << "변경된 벤더 유형: " << agentA->getVendorType() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "오류 발생: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
