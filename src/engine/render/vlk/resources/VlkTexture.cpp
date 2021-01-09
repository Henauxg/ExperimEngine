#include "VlkTexture.hpp"

#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkMemoryAllocator.hpp>
#include <engine/render/vlk/VlkTools.hpp>

namespace expengine {
namespace render {
namespace vlk {

Texture::Texture(
    const vlk::Device& device,
    void* texData,
    vk::DeviceSize bufferSize,
    vk::Format format,
    uint32_t texWidth,
    uint32_t texHeight,
    const vk::Sampler sampler,
    vk::ImageUsageFlags imageUsageFlags,
    vk::ImageLayout targetImgLayout)
    : width_(texWidth)
    , height_(texHeight)
    , sampler_(sampler)
    , mipLevels_(1)
    , layerCount_(1)
{
    /* Upload texData to accessible device memory */
    auto stagingBuffer = device.allocator().createStagingBuffer(bufferSize, texData);

    /* TODO : consider linear tiling if there is no other way */
    /* Create image (as a transfer dest) */
    vk::ImageCreateInfo imgCreateInfo
        = {.imageType = vk::ImageType::e2D,
           .format = format,
           .extent = {width_, height_, 1},
           .mipLevels = mipLevels_,
           .arrayLayers = layerCount_,
           .samples = vk::SampleCountFlagBits::e1,
           .tiling = vk::ImageTiling::eOptimal,
           .usage = (imageUsageFlags | vk::ImageUsageFlagBits::eTransferDst),
           .sharingMode = vk::SharingMode::eExclusive,
           .initialLayout = vk::ImageLayout::eUndefined};

    auto createImgResult = device.deviceHandle().createImageUnique(imgCreateInfo);
    EXPENGINE_VK_ASSERT(createImgResult.result, "Failed to create image");
    image_ = std::move(createImgResult.value);

    /* Allocate memory for the image (device local) and bind it */
    auto memRequirements
        = device.deviceHandle().getImageMemoryRequirements(image_.get());
    vk::MemoryAllocateInfo memAllocInfo {
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = device.findMemoryType(
            memRequirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eDeviceLocal)};
    auto memAlloc = device.deviceHandle().allocateMemoryUnique(memAllocInfo);
    EXPENGINE_VK_ASSERT(memAlloc.result, "Failed to allocate memory for an image.");
    memory_ = std::move(memAlloc.value);

    auto bindResult
        = device.deviceHandle().bindImageMemory(image_.get(), memory_.get(), 0);
    EXPENGINE_VK_ASSERT(bindResult, "Failed to bind memory to an image.");

    auto imageCopyCmdBuffer = device.createTransientCommandBuffer();

    /* Transition image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL */
    vlk::transitionImageLayout(
        imageCopyCmdBuffer.getHandle(),
        image_.get(),
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageAspectFlagBits::eColor);

    /* Copy stagingBuffer to image */
    vk::BufferImageCopy bufferCopyRegion {
        .imageSubresource
        = {.aspectMask = vk::ImageAspectFlagBits::eColor,
           .mipLevel = 0,
           .baseArrayLayer = 0,
           .layerCount = 1},
        .imageExtent = {.width = width_, .height = height_, .depth = 1}};
    imageCopyCmdBuffer.copyBufferToImage(
        stagingBuffer->getHandle(), image_.get(), bufferCopyRegion);

    /* Transition layout to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL */
    vlk::transitionImageLayout(
        imageCopyCmdBuffer.getHandle(),
        image_.get(),
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ImageAspectFlagBits::eColor);
    /* Update texture info */
    layout_ = vk::ImageLayout::eShaderReadOnlyOptimal;

    /* TODO Implement fence signaling in buffer sumbission */
    device.submitTransientCommandBuffer(imageCopyCmdBuffer);

    /* Create Image View */
    auto createViewResult = device.deviceHandle().createImageViewUnique(
        {.image = image_.get(),
         .viewType = vk::ImageViewType::e2D,
         .format = format,
         .subresourceRange
         = {.aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1}});
    EXPENGINE_VK_ASSERT(createViewResult.result, "Failed to create image view");
    view_ = std::move(createViewResult.value);

    /* Update descriptor */
    descriptorInfo_.sampler = sampler;
    descriptorInfo_.imageView = view_.get();
    descriptorInfo_.imageLayout = layout_;
}

} // namespace vlk
} // namespace render
} // namespace expengine
