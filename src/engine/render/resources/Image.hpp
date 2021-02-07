#pragma once

#include <optional>
#include <string>

#include <engine/render/Color.hpp>

namespace experim {

/**
 * 4 channels : RGBA, 8 bits per channel
 */
class Image {
public:
    Image();
    ~Image();

    static std::optional<Image> fromFile(const std::string& filepath);
    static std::optional<Image> fromBuffer(
        const uint8_t* buffer,
        uint32_t bufferSize);

    inline const std::pair<uint32_t, uint32_t> size() { return size_; }

    const Color getPixelColor(uint32_t x, uint32_t y) const;

private:
    uint8_t* data_;
    std::pair<uint32_t, uint32_t> size_;
};

} // namespace experim
