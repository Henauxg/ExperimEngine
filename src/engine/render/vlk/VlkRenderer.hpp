#pragma once

#include <SDL2\SDL_events.h>

#include <engine/render/Renderer.hpp>
#include <engine/render/imgui/impl/ImGuiBackend.hpp>
#include <engine/render/vlk/RenderingContext.hpp>
#include <engine/render/vlk/VlkMemoryAllocator.hpp>
#include <engine/render/vlk/VlkWindow.hpp>

namespace expengine {
namespace render {
namespace vlk {

class vlk::Device;

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer(
        const std::string& appName,
        const uint32_t appVersion,
        int windowWidth,
        int windoHeight,
        EngineParameters& engineParams);
    ~VulkanRenderer();

    void render();
    void handleEvent(const SDL_Event& event);
    void rendererWaitIdle();
    std::shared_ptr<Window> getMainWindow();

private:
    /* Vulkan objects */
    vk::UniqueInstance vkInstance_;
    std::shared_ptr<vlk::VulkanWindow> mainWindow_;
    std::unique_ptr<vlk::Device> vlkDevice_;
    std::unique_ptr<vlk::MemoryAllocator> memAllocator_;
    std::shared_ptr<RenderingContext> mainRenderingContext_;

    /* UI */
    std::unique_ptr<ImguiBackend> imguiBackend_;

    /* TODO : vk::UniqueDebugUtilsMessengerEXT */
    /**  @brief Only used in debug mode. */
    vk::DebugUtilsMessengerEXT vkDebugMessenger_;

    vk::UniqueInstance createVulkanInstance(
        const std::string& appName,
        const uint32_t appVersion,
        const vlk::VulkanWindow& window) const;
    vk::DebugUtilsMessengerEXT setupDebugMessenger(
        vk::Instance instance,
        bool enableValidationLayers) const;
};

} // namespace vlk
} // namespace render
} // namespace expengine
