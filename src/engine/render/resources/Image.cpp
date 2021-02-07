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

Image::~Image()
{
    if (data_)
    {
        delete[] data_;
    }
}

std::pair<bool, std::unique_ptr<Image>> Image::fromFile(const std::string& filepath)
{
    int w, h = 0;
    int channels;
    unsigned char* pData = stbi_load(filepath.c_str(), &w, &h, &channels, 4);

    if (pData == nullptr)
    {
        return std::make_pair(false, nullptr);
    }

    auto image = std::make_unique<Image>();
    image->size_ = {w, h};
    image->data_ = pData;
    return std::make_pair(true, std::move(image));
}

std::pair<bool, std::unique_ptr<Image>> Image::fromBuffer(
    const uint8_t* buffer,
    uint32_t bufferSize)
{
    int w, h = 0;
    int channels;
    unsigned char* pData
        = stbi_load_from_memory(buffer, bufferSize, &w, &h, &channels, 4);
    if (pData == nullptr)
    {
        return std::make_pair(false, nullptr);
    }

    auto image = std::make_unique<Image>();
    image->size_ = {w, h};
    image->data_ = pData;
    return std::make_pair(true, std::move(image));
}

const Color Image::getPixelColor(uint32_t x, uint32_t y) const
{
    uint32_t index = x + y * size_.first;
    EXPENGINE_DEBUG_ASSERT(
        index < size_.first * size_.second, "Requested pixel out of image");

    return Color(data_[index]);
}

} // namespace experim
