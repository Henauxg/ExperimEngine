#pragma once

#include <SDL2\SDL_events.h>

#include <engine/render/Renderer.hpp>
#include <engine/render/vlk/VlkDevice.hpp>

namespace experim {

class ImguiBackend;

namespace vlk {

class Device;
class VulkanWindow;
class MemoryAllocator;
class VulkanRenderingContext;

class VulkanRenderer final : public Renderer {
public:
    VulkanRenderer(
        const std::string& appName,
        const uint32_t appVersion,
        int windowWidth,
        int windoHeight,
        EngineParameters& engineParams);

    ~VulkanRenderer() override;

    void handleEvent(const SDL_Event& event) override;
    void prepareFrame() override;
    void renderFrame() override;

    void waitIdle() override;
    std::shared_ptr<Window> getMainWindow() override;

    inline const vlk::Device& getDevice() const { return *vlkDevice_; };

private:
    /* Vulkan objects */
    vk::UniqueInstance vkInstance_;
    std::shared_ptr<VulkanWindow> mainWindow_;
    std::unique_ptr<vlk::Device> vlkDevice_;
    std::shared_ptr<vlk::VulkanRenderingContext> mainRenderingContext_;

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
} // namespace experim
