#ifndef MODALITY_HPP
#define MODALITY_HPP

#include <string>
#include <vector>
#include <optional>
#include <memory>

/**
 * @brief 텍스트와 이미지를 담을 수 있는 멀티모달 클래스
 */
class Modality {
public:
    Modality() = default;
    ~Modality() = default;

    // 텍스트 관련 메서드
    void addText(const std::string& text);
    void clearText();
    bool hasText() const;
    std::string getText() const;

    // 이미지 관련 메서드
    void addImagePath(const std::string& path);
    void addImageData(const std::vector<uint8_t>& data, const std::string& mimeType = "image/jpeg");
    void clearImage();
    bool hasImage() const;
    bool hasImagePath() const;
    bool hasImageData() const;
    std::string getImagePath() const;
    const std::vector<uint8_t>& getImageData() const;
    std::string getImageMimeType() const;

    // 직렬화/역직렬화 메서드
    std::string serialize() const;
    static Modality deserialize(const std::string& data);

private:
    std::optional<std::string> text;
    std::optional<std::string> imagePath;
    std::optional<std::vector<uint8_t>> imageData;
    std::string imageMimeType;
};

#endif // MODALITY_HPP
