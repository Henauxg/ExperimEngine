#pragma once

#include <vector>

#include <engine/render/Window.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

class VulkanWindow final : public Window {
public:
    VulkanWindow(int width, int height, const std::string& title);
    VulkanWindow(int width, int height, const std::string& title, uint32_t flags);
    /** Used to create a dummy vulkan window. Used in vlk::Device constructor. */
    VulkanWindow();

    /* Vulkan only */
    std::pair<bool, vk::SurfaceKHR> createVkSurface(vk::Instance vkInstance) const;
    std::vector<const char*> getRequiredVkExtensions() const;

    /* Window virtual methods implementation */
    std::pair<uint32_t, uint32_t> getDrawableSizeInPixels() const override;
    std::shared_ptr<Window> clone(
        int width,
        int height,
        const std::string& title,
        uint32_t flags) override;
};

} // namespace vlk
} // namespace render
} // namespace expengine
