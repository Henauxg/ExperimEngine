#include "VlkTexture.hpp"

#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkMemoryAllocator.hpp>
#include <engine/render/vlk/resources/VlkImage.hpp>

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
    : sampler_(sampler)
{
    /* Upload texData to accessible device memory */
    auto stagingBuffer = device.allocator().createStagingBuffer(bufferSize, texData);

    /* Create image (as a transfer dest) */
    image_ = device.allocator().createTextureImage(
        imageUsageFlags | vk::ImageUsageFlagBits::eTransferDst,
        format,
        texWidth,
        texHeight);

    auto imageCopyCmdBuffer = device.createTransientCommandBuffer();

    /* Transition image layout to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL */
    image_->transitionImageLayout(
        imageCopyCmdBuffer.getHandle(),
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
        .imageExtent = image_->getExtent()};
    imageCopyCmdBuffer.copyBufferToImage(
        stagingBuffer->getHandle(), image_->getHandle(), bufferCopyRegion);

    /* Transition layout (defautl to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) */
    image_->transitionImageLayout(
        imageCopyCmdBuffer.getHandle(),
        vk::ImageLayout::eTransferDstOptimal,
        targetImgLayout,
        vk::ImageAspectFlagBits::eColor);

    /* TODO Implement fence signaling in buffer sumbission */
    device.submitTransientCommandBuffer(imageCopyCmdBuffer);

    /* Create Image View */
    auto createViewResult = device.deviceHandle().createImageViewUnique(
        {.image = image_->getHandle(),
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
    descriptorInfo_.imageLayout = image_->getLayout();
}

} // namespace vlk
} // namespace render
} // namespace expengine
