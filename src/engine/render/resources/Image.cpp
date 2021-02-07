#include "Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <engine/log/ExpengineLog.hpp>

namespace experim {

Image::Image()
    : data_(nullptr)
    , size_ {0, 0}
{
}

Image::~Image() { stbi_image_free(data_); }

std::optional<Image> Image::fromFile(const std::string& filepath)
{
    int w, h = 0;
    int channels;
    auto pixels = stbi_load(filepath.c_str(), &w, &h, &channels, 4);
    if (pixels == nullptr)
    {
        return std::optional<Image>();
    }

    Image image;
    image.size_ = {w, h};
    image.data_ = static_cast<uint8_t*>(pixels);
    return std::optional<Image>(image);
}

std::optional<Image> Image::fromBuffer(const uint8_t* buffer, uint32_t bufferSize)
{
    int w, h = 0;
    int channels;
    auto pixels = stbi_load_from_memory(buffer, bufferSize, &w, &h, &channels, 4);
    if (pixels == nullptr)
    {
        return std::optional<Image>();
    }

    Image image;
    image.size_ = {w, h};
    image.data_ = static_cast<uint8_t*>(pixels);
    return std::optional<Image>(image);
}

const Color Image::getPixelColor(uint32_t x, uint32_t y) const
{
    uint32_t index = x + y * size_.first;
    EXPENGINE_DEBUG_ASSERT(index < size_.first * size_.second, "");

    return Color(data_[index]);
}

} // namespace experim
