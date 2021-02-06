#include "WGpuTexture.hpp"

namespace experim {
namespace webgpu {

Texture::Texture(
    const wgpu::Device device,
    void* texData,
    size_t texDataSize,
    wgpu::TextureFormat format,
    uint32_t texWidth,
    uint32_t texHeight,
    uint32_t texelSize,
    const char* label,
    wgpu::TextureUsage usage)
{
    /* Create texture and texture view */
    wgpu::TextureDescriptor textureDesc {
        .label = label,
        .usage = wgpu::TextureUsage::CopyDst | usage,
        .dimension = wgpu::TextureDimension::e2D,
        .size = {.width = texWidth, .height = texHeight, .depth = 1},
        .format = format,
        .mipLevelCount = 1,
        .sampleCount = 1};
    texture_ = device.CreateTexture(&textureDesc);

    wgpu::TextureViewDescriptor textureViewDesc {
        .format = format,
        .dimension = wgpu::TextureViewDimension::e2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspect = wgpu::TextureAspect::All};
    textureView_ = texture_.CreateView(&textureViewDesc);

    /* Create staging buffer */
    wgpu::BufferDescriptor stagingBufferDesc {
        .usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst,
        .size = texDataSize};
    wgpu::Buffer stagingBuffer = device.CreateBuffer(&stagingBufferDesc);
    auto queue = device.GetDefaultQueue();
    queue.WriteBuffer(stagingBuffer, 0, texData, texDataSize);

    /* Copy to texture memory */
    wgpu::BufferCopyView bufferCopyView {
        .layout = {.bytesPerRow = texWidth * texelSize, .rowsPerImage = texHeight},
        .buffer = stagingBuffer};
    wgpu::TextureCopyView textureCopyView {.texture = texture_};
    wgpu::Extent3D copySize {texWidth, texHeight, 1};

    auto encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToTexture(&bufferCopyView, &textureCopyView, &copySize);

    auto commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);
}

} // namespace webgpu
} // namespace experim
