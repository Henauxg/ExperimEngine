#pragma once

#include <webgpu/webgpu_cpp.h>

namespace experim {
namespace webgpu {

class WgpuTexture {
public:
    /** Create texture from data buffer */
    WgpuTexture(
        const wgpu::Device device,
        void* texData,
        size_t texDataSize,
        wgpu::TextureFormat format,
        uint32_t texWidth,
        uint32_t texHeight,
        uint32_t texelSize = 4,
        const char* label = nullptr,
        wgpu::TextureUsage usage = wgpu::TextureUsage::Sampled);

    inline wgpu::Texture const texureHandle() const { return texture_; };
    inline wgpu::TextureView const viewHandle() const { return textureView_; };

private:
    /* Owned objects */
    wgpu::Texture texture_;
    wgpu::TextureView textureView_;
};

} // namespace webgpu
} // namespace experim
