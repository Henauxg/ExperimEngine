#pragma once

#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkInclude.hpp>
#include <engine/render/vlk/resources/VlkImage.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Image;
class Device;

class Texture {
public:
    /** Create texture from data buffer */
    Texture(
        const vlk::Device& device,
        void* texData,
        vk::DeviceSize texDataSize,
        vk::Format format,
        uint32_t texWidth,
        uint32_t texHeight,
        const vk::Sampler sampler,
        vk::ImageUsageFlags imageUsageFlags = vk::ImageUsageFlagBits::eSampled,
        vk::ImageLayout targetImgLayout = vk::ImageLayout::eShaderReadOnlyOptimal);

    inline vk::Image imageHandle() const { return image_->getHandle(); };
    inline const vk::DescriptorImageInfo& descriptorInfo() const
    {
        return descriptorInfo_;
    }

private:
    /* Handles */
    /* A sampler may be shared with multiples textures */
    /* TODO may create default sampler is none is provided ? */
    const vk::Sampler sampler_;

    /* Owned objects */
    std::unique_ptr<Image> image_;
    vk::UniqueImageView view_;

    /* Info */
    vk::DescriptorImageInfo descriptorInfo_;
};
} // namespace vlk
} // namespace render
} // namespace expengine
