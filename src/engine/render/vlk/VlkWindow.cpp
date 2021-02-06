#include "VlkWindow.hpp"

#include <SDL2/SDL_vulkan.h>

#include <engine/log/ExpengineLog.hpp>

namespace experim {
namespace vlk {

VulkanWindow::VulkanWindow(
    int width,
    int height,
    const std::string& title,
    uint32_t flags)
    : Window(width, height, title, flags | SDL_WINDOW_VULKAN)
{
}

VulkanWindow::VulkanWindow(int width, int height, const std::string& title)
    : Window(
        width,
        height,
        title,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI)
{
}

VulkanWindow::VulkanWindow()
    : Window(
        100,
        100,
        "No title",
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
            | SDL_WINDOW_HIDDEN)
{
}

std::shared_ptr<Window> VulkanWindow::clone(
    int width,
    int height,
    const std::string& title,
    uint32_t flags)
{
    auto vlkWindow
        = std::make_shared<vlk::VulkanWindow>(width, height, title, flags);
    return std::static_pointer_cast<Window>(vlkWindow);
}

std::pair<bool, vk::SurfaceKHR> VulkanWindow::createVkSurface(
    vk::Instance vkInstance) const
{
    vk::SurfaceKHR surface;
    bool result
        = SDL_Vulkan_CreateSurface(sdlWindow_, vkInstance, (VkSurfaceKHR*) &surface);

    return {result, surface};
}

std::vector<const char*> VulkanWindow::getRequiredVkExtensions() const
{
    uint32_t extensionCount;
    /* TODO Error handling since we will create windows on the fly */
    EXPENGINE_ASSERT(
        SDL_Vulkan_GetInstanceExtensions(sdlWindow_, &extensionCount, nullptr),
        "Failed to get the count of required Vulkan extensions "
        "by the SDL window");

    std::vector<const char*> windowExtensions(extensionCount);
    /* TODO Error handling since we will create windows on the fly */
    EXPENGINE_ASSERT(
        SDL_Vulkan_GetInstanceExtensions(
            sdlWindow_, &extensionCount, windowExtensions.data()),
        "Failed to get the names of the Vulkan extensions required by the "
        "SDL window");

    return windowExtensions;
}

std::pair<uint32_t, uint32_t> VulkanWindow::getDrawableSizeInPixels() const
{
    int w = 0, h = 0;
    SDL_Vulkan_GetDrawableSize(sdlWindow_, &w, &h);
    return {(uint32_t) w, (uint32_t) h};
}

} // namespace vlk
} // namespace experim
