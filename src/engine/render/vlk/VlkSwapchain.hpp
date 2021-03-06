#pragma once

#include <vector>

#include <engine/render/vlk/VlkInclude.hpp>

namespace spdlog {
class logger;
}

namespace experim {
namespace vlk {

class Device;

class Swapchain {
public:
    Swapchain(
        const vlk::Device& device,
        vk::SurfaceKHR& surface,
        vk::Extent2D requestedExtent,
        vk::SwapchainKHR oldSwapchainHandle = nullptr);
    ~Swapchain();

    inline const vk::SwapchainKHR getHandle() const { return *swapchain_; }
    inline const vk::Extent2D& getImageExtent() const { return imageExtent_; }
    inline const vk::Extent2D& getRequestedExtent() const
    {
        return requestedExtent_;
    }
    inline const vk::SurfaceFormatKHR& getSurfaceFormat() const
    {
        return surfaceFormat_;
    }
    inline const uint32_t getImageCount() const
    {
        return static_cast<uint32_t>(images_.size());
    }
    inline const std::vector<vk::Image>& getImages() const { return images_; }

    vk::ResultValue<uint32_t> acquireNextImage(
        vk::Semaphore& imageAcquiredSemaphore);
    vk::Result presentImage(
        vk::Queue presentQueue,
        uint32_t imageIndex,
        const vk::Semaphore& waitSemaphore);

private:
    /* References */
    const vlk::Device& device_;
    const vk::SurfaceKHR& surface_;

    vk::UniqueSwapchainKHR swapchain_;
    /* Swapchain properties */
    vk::SurfaceFormatKHR surfaceFormat_;
    vk::PresentModeKHR presentMode_;
    vk::Extent2D imageExtent_;
    /* Extent that was requested when creating the swapchain. May not be the same as
     * the actual extent (imageExtent_) */
    vk::Extent2D requestedExtent_;

    /* Swapchain objects  */
    std::vector<vk::Image> images_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(
        const vk::SurfaceFormatKHR requestedFormat,
        const std::vector<vk::SurfaceFormatKHR>& availableFormats,
        const std::vector<vk::SurfaceFormatKHR>& formatsPriorityList) const;
    vk::PresentModeKHR chooseSwapPresentMode(
        vk::PresentModeKHR requestedMode,
        const std::vector<vk::PresentModeKHR> availablePresentModes,
        const std::vector<vk::PresentModeKHR>& presentModePriorityList) const;
    vk::Extent2D chooseSwapExtent(
        vk::Extent2D requestedExtent,
        vk::Extent2D currentExtent,
        vk::Extent2D minImageExten,
        vk::Extent2D maxImageExtent) const;
};

} // namespace vlk
} // namespace experim
