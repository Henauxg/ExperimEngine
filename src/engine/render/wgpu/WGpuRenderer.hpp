#pragma once

#include <SDL2\SDL_events.h>

#include <webgpu/webgpu_cpp.h>

#include <engine/render/Renderer.hpp>
#include <engine/render/Window.hpp>

namespace experim {

class ImguiBackend;
class Texture;

namespace webgpu {

class WebGpuRenderingContext;

class WebGpuRenderer final : public Renderer {
public:
    WebGpuRenderer(
        const std::string& appName,
        const uint32_t appVersion,
        int windowWidth,
        int windoHeight,
        EngineParameters& engineParams);

    ~WebGpuRenderer() override;

    inline const wgpu::Device& device() const { return device_; };

    bool handleEvent(const SDL_Event& event) override;
    void prepareFrame() override;
    void renderFrame() override;

    void waitIdle() override;
    std::shared_ptr<Window> getMainWindow() const override;

    /* Implement IRendering */
    std::unique_ptr<Texture> createTexture() override;

private:
    std::shared_ptr<Window> mainWindow_;

    /* WebGPU objects */
    wgpu::Device device_;
    wgpu::Queue queue_;
    std::shared_ptr<WebGpuRenderingContext> mainRenderingContext_;

    /* UI */
    std::unique_ptr<ImguiBackend> imguiBackend_;
};

} // namespace webgpu
} // namespace experim
