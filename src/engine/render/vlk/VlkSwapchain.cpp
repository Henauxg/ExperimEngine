#include "VlkSwapchain.hpp"

#include <algorithm>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkDevice.hpp>

namespace {

/* Preferred surface formats (from most to least) */
const std::vector<vk::SurfaceFormatKHR> SURFACE_FORMATS_PRIORITY_LIST
    = {{vk::Format::eR8G8B8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
       {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
       {vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear},
       {vk::Format::eR8G8B8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear},
       {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear}};

/* Preferred present modes (from most to least).
 * VK_PRESENT_MODE_MAILBOX_KHR allow for triple buffering and limited
 * latency.
 * VK_PRESENT_MODE_IMMEDIATE_KHR for minimal latency
 * VK_PRESENT_MODE_FIFO_KHR double buffering with some latency
 */
std::vector<vk::PresentModeKHR> PRESENT_MODE_PRIORITY_LIST
    = {vk::PresentModeKHR::eMailbox,
       vk::PresentModeKHR::eImmediate,
       vk::PresentModeKHR::eFifo};

} // namespace

namespace expengine {
namespace render {
namespace vlk {

Swapchain::Swapchain(
    const vlk::Device& device,
    vk::SurfaceKHR& surface,
    vk::Extent2D requestedExtent,
    vk::SwapchainKHR oldSwapchainHandle)
    : device_(device)
    , surface_(surface)
    , requestedExtent_(requestedExtent)
    , logger_(spdlog::get(LOGGER_NAME))
{
    /* Check for WSI support */
    EXPENGINE_ASSERT(
        device.getSurfaceSupport(surface),
        "Surface/WSI not supported by physical device");

    /* Query the device swapchain properties for this surface */
    auto swapchainSupport = device.querySwapChainSupport(surface);

    /* Select the best Surface Format available */
    /* Simply request SURFACE_FORMATS_PRIORITY_LIST[0] as the preferred one
     * for now */
    surfaceFormat_ = chooseSwapSurfaceFormat(
        SURFACE_FORMATS_PRIORITY_LIST[0],
        swapchainSupport.formats,
        SURFACE_FORMATS_PRIORITY_LIST);
    /* Select the best Present Mode available */
    /* Request mailbox for now but should enable Vsync toggle */
    presentMode_ = chooseSwapPresentMode(
        vk::PresentModeKHR::eMailbox,
        swapchainSupport.presentModes,
        PRESENT_MODE_PRIORITY_LIST);
    /* Select the Swap Extent */
    imageExtent_ = chooseSwapExtent(
        requestedExtent,
        swapchainSupport.capabilities.currentExtent,
        swapchainSupport.capabilities.minImageExtent,
        swapchainSupport.capabilities.maxImageExtent);

    /* Chose min image count */
    uint32_t requestedMinImageCount
        = swapchainSupport.capabilities.minImageCount + 1;
    /* A value of 0 for maxImageCount means that there is no limit besides
     * memory requirements. */
    if (swapchainSupport.capabilities.maxImageCount > 0
        && requestedMinImageCount > swapchainSupport.capabilities.maxImageCount)
    {
        requestedMinImageCount = swapchainSupport.capabilities.maxImageCount;
    }

    /* Chose image sharing mode */
    QueueFamilyIndices queueIndices = device.queueIndices();
    uint32_t queueFamilyIndices[]
        = {queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
    /* This case offers non-optimal performances but should not happen, see
     * findQueueFamilies in VlkCapabilities.cpp for an explanation  */
    vk::SharingMode imageSharingMode = vk::SharingMode::eExclusive;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
    if (queueIndices.graphicsFamily != queueIndices.presentFamily)
    {
        imageSharingMode = vk::SharingMode::eConcurrent;
        queueFamilyIndexCount = 2;
        pQueueFamilyIndices = queueFamilyIndices;
    }

    /* Create SwapChain */
    vk::SwapchainCreateInfoKHR createInfo {
        .surface = surface,
        .minImageCount = requestedMinImageCount,
        .imageFormat = surfaceFormat_.format,
        .imageColorSpace = surfaceFormat_.colorSpace,
        .imageExtent = imageExtent_,
        .imageArrayLayers = 1,
        /* Render directly, no post-processsing */
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = imageSharingMode,
        .queueFamilyIndexCount = queueFamilyIndexCount,
        .pQueueFamilyIndices = pQueueFamilyIndices,
        /* No transformation */
        .preTransform = swapchainSupport.capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode_,
        .clipped = VK_TRUE,
        .oldSwapchain = oldSwapchainHandle};

    auto swapchainCreateResult
        = device.deviceHandle().createSwapchainKHRUnique(createInfo);
    EXPENGINE_VK_ASSERT(swapchainCreateResult.result, "Failed to create Swapchain");
    swapchain_ = std::move(swapchainCreateResult.value);

    /* Retrieve images from the swapchain */
    vk::Result result;
    std::tie(result, images_)
        = device_.deviceHandle().getSwapchainImagesKHR(swapchain_.get());
    EXPENGINE_VK_ASSERT(result, "Failed to retrieve swapchain images");
}

Swapchain::~Swapchain() { SPDLOG_LOGGER_DEBUG(logger_, "Swapchain destruction"); }

vk::ResultValue<uint32_t> Swapchain::acquireNextImage(
    vk::Semaphore& imageAcquiredSemaphore)
{
    return device_.deviceHandle().acquireNextImageKHR(
        *swapchain_,
        std::numeric_limits<uint64_t>::max(),
        imageAcquiredSemaphore,
        nullptr);
}

vk::Result Swapchain::presentImage(
    vk::Queue presentQueue,
    uint32_t imageIndex,
    const vk::Semaphore& waitSemaphore)
{
    vk::PresentInfoKHR presInfo {
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &waitSemaphore,
        .swapchainCount = 1,
        .pSwapchains = &swapchain_.get(),
        .pImageIndices = &imageIndex};
    /* Explicitly give a pointer to use the "non-enhanced" function. This enable
     * custom error code handling. */
    return presentQueue.presentKHR(&presInfo);
}

/* Based on
 * https://github.com/KhronosGroup/Vulkan-Samples/blob/master/framework/core/swapchain.cpp
 */
vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(
    const vk::SurfaceFormatKHR requestedFormat,
    const std::vector<vk::SurfaceFormatKHR>& availableFormats,
    const std::vector<vk::SurfaceFormatKHR>& formatsPriorityList) const
{
    /* The best case scenario is that the surface has no preferred format,
     * which Vulkan indicates by only returning one VkSurfaceFormatKHR
     * entry which has its format member set to VK_FORMAT_UNDEFINED */
    if (availableFormats.size() == 1
        && availableFormats[0].format == vk::Format::eUndefined)
    {
        SPDLOG_LOGGER_DEBUG(
            logger_,
            "Swapchain : surface format '{} ; {}' selected",
            requestedFormat.format,
            requestedFormat.colorSpace);
        return requestedFormat;
    }

    /* Try to find the requested format in the supported surface formats */
    auto surfaceFormatIterator = std::find_if(
        availableFormats.begin(),
        availableFormats.end(),
        [&requestedFormat](const vk::SurfaceFormatKHR& comparedFormat) {
            if (comparedFormat.format == requestedFormat.format
                && comparedFormat.colorSpace == requestedFormat.colorSpace)
            {
                return true;
            }
            return false;
        });
    /* If the requested surface format isn't found, then try to request a
     * format from the priority list */
    if (surfaceFormatIterator == availableFormats.end())
    {
        bool found = false;
        for (auto& surfaceFormat : formatsPriorityList)
        {
            surfaceFormatIterator = std::find_if(
                availableFormats.begin(),
                availableFormats.end(),
                [&surfaceFormat](const vk::SurfaceFormatKHR& comparedFormat) {
                    if (comparedFormat.format == surfaceFormat.format
                        && comparedFormat.colorSpace == surfaceFormat.colorSpace)
                    {
                        return true;
                    }
                    return false;
                });
            if (surfaceFormatIterator != availableFormats.end())
            {
                found = true;
                break;
            }
        }

        /* If nothing is found, default to the first supported surface
         * format */
        if (!found)
        {
            surfaceFormatIterator = availableFormats.begin();
        }
        SPDLOG_LOGGER_WARN(
            logger_,
            "Swapchain : surface format '{} ; {}' not "
            "supported, selecting '{} ; {}'",
            requestedFormat.format,
            requestedFormat.colorSpace,
            (*surfaceFormatIterator).format,
            (*surfaceFormatIterator).colorSpace);
    }
    else
    {
        /* The requested surface format was found */
        SPDLOG_LOGGER_DEBUG(
            logger_,
            "Swapchain : surface format '{} ; {}' selected",
            requestedFormat.format,
            requestedFormat.colorSpace);
    }

    return *surfaceFormatIterator;
}

vk::PresentModeKHR Swapchain::chooseSwapPresentMode(
    vk::PresentModeKHR requestedMode,
    const std::vector<vk::PresentModeKHR> availablePresentModes,
    const std::vector<vk::PresentModeKHR>& presentModePriorityList) const
{
    auto presentModeIterator = std::find(
        availablePresentModes.begin(), availablePresentModes.end(), requestedMode);

    if (presentModeIterator == availablePresentModes.end())
    {
        /* If nothing found, always default to FIFO which is always
         * available */
        vk::PresentModeKHR selectedMode = vk::PresentModeKHR::eFifo;
        for (auto& presentMode : presentModePriorityList)
        {
            if (std::find(
                    availablePresentModes.begin(),
                    availablePresentModes.end(),
                    presentMode)
                != availablePresentModes.end())
            {
                selectedMode = presentMode;
            }
        }
        SPDLOG_LOGGER_WARN(
            logger_,
            "Swapchain : present mode '{}' not supported, selecting '{}'",
            requestedMode,
            selectedMode);
        return selectedMode;
    }
    else
    {
        SPDLOG_LOGGER_DEBUG(
            logger_,
            "Swapchain : present mode selected '{}' selected",
            requestedMode);
        return *presentModeIterator;
    }

    return vk::PresentModeKHR();
}

vk::Extent2D Swapchain::chooseSwapExtent(
    vk::Extent2D requestedExtent,
    vk::Extent2D currentExtent,
    vk::Extent2D minImageExten,
    vk::Extent2D maxImageExtent) const
{
    vk::Extent2D swapchainExtent = {};
    /* The special value (0xFFFFFFFF, 0xFFFFFFFF) indicates that the
     * surface size will be determined by the extent of a swapchain
     * targeting the surface */
    if (currentExtent.width == (uint32_t) 0xFFFFFFFF)
    {
        swapchainExtent.width = std::clamp(
            requestedExtent.width, minImageExten.width, maxImageExtent.width);
        swapchainExtent.height = std::clamp(
            requestedExtent.height, minImageExten.height, maxImageExtent.height);
    }
    else
    {
        /* If the surface size is defined, the swap chain size must match
         */
        swapchainExtent = currentExtent;
    }

    SPDLOG_LOGGER_DEBUG(
        logger_,
        "Swapchain : extent '{};{}' selected, requested '{};{}'",
        swapchainExtent.width,
        swapchainExtent.height,
        requestedExtent.width,
        requestedExtent.height);
    return swapchainExtent;
}

} // namespace vlk
} // namespace render
} // namespace expengine
