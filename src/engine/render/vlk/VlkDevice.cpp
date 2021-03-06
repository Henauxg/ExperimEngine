#include "VlkDevice.hpp"

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkWindow.hpp>

namespace experim {
namespace vlk {

const std::vector<const char*> DEVICE_EXTENSIONS = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

Device::Device(
    vk::Instance vkInstance,
    const vk::DispatchLoaderDynamic& dispatchLoader,
    std::shared_ptr<spdlog::logger> logger)
    : vkInstance_(vkInstance)
    , logger_(logger)
{
    /* Create a temporary dummy window/surface to get information
     * about surface compatibility between the Vulkan devices
     * selected/created and the surfaces that will be in use during the
     * application runtime. */
    SPDLOG_LOGGER_DEBUG(
        logger_, "Dummy window creation for surface compatibility checks");
    auto dummyWindow = std::make_unique<VulkanWindow>();
    auto [surfaceCreated, dummySurface] = dummyWindow->createVkSurface(vkInstance);
    EXPENGINE_ASSERT(surfaceCreated, "Failed to create a VkSurface");
    auto windowSurface_ = vk::UniqueSurfaceKHR(dummySurface, vkInstance);

    /* Devices */
    physDevice_ = pickPhysicalDevice(
        vkInstance,
        DEVICE_EXTENSIONS,
        std::vector<vk::SurfaceKHR> {*windowSurface_});
    logicalDevice_ = createLogicalDevice(
        physDevice_.device, physDevice_.queuesIndices, DEVICE_EXTENSIONS);

    /* Memory allocator */
    memAllocator_ = std::make_unique<vlk::MemoryAllocator>(
        vkInstance_, *this, dispatchLoader, ENGINE_VULKAN_API_VERSION);

    /* Queues handles */
    graphicsQueue_ = logicalDevice_->getQueue(
        physDevice_.queuesIndices.graphicsFamily.value(), 0);
    presentQueue_ = logicalDevice_->getQueue(
        physDevice_.queuesIndices.presentFamily.value(), 0);

    /* Descriptor pool */
    descriptorPool_ = createDescriptorPool();

    /* Command pool for short lived buffers */
    auto cmdPoolResult = logicalDevice_->createCommandPoolUnique(
        {.flags = vk::CommandPoolCreateFlagBits::eTransient,
         .queueFamilyIndex = physDevice_.queuesIndices.graphicsFamily.value()});
    EXPENGINE_VK_ASSERT(
        cmdPoolResult.result, "Failed to create the device transient command pool");
    transientCommandPool_ = std::move(cmdPoolResult.value);
}

Device::~Device() { SPDLOG_LOGGER_DEBUG(logger_, "Device destruction"); }

const SwapChainSupportDetails Device::querySwapChainSupport(
    vk::SurfaceKHR& surface) const
{
    return queryPhysicalDeviceSwapChainSupport(physDevice_.device, surface);
}

VkBool32 Device::getSurfaceSupport(vk::SurfaceKHR& surface) const
{
    auto support = physDevice_.device.getSurfaceSupportKHR(
        queueIndices().presentFamily.value(), surface);
    EXPENGINE_VK_ASSERT(support.result, "Failed to check for surface support");
    return support.value;
}

vk::ResultValue<vk::SurfaceCapabilitiesKHR> Device::getSurfaceCapabilities(
    vk::SurfaceKHR& surface) const
{
    return physDevice_.device.getSurfaceCapabilitiesKHR(surface);
}

const CommandBuffer Device::createTransientCommandBuffer() const
{
    CommandBuffer commandBuffer(*this, transientCommandPool_.get());
    commandBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    return commandBuffer;
}

const void Device::submitTransientCommandBuffer(CommandBuffer& commandBuffer) const
{
    commandBuffer.end();

    auto handle = commandBuffer.getHandle();
    vk::SubmitInfo submitInfo {.commandBufferCount = 1, .pCommandBuffers = &handle};
    /* TODO : Could implement fences for transient buffers ? */
    auto res = graphicsQueue_.submit(submitInfo, nullptr);
    EXPENGINE_VK_ASSERT(
        res, "Failed to submit transient command buffer to graphics queue");

    res = graphicsQueue_.waitIdle();
    EXPENGINE_VK_ASSERT(res, "Failed to wait on the graphics queue to be idle");
}

void Device::waitIdle() const
{
    auto res = logicalDevice_->waitIdle();
    EXPENGINE_VK_ASSERT(res, "Failed to wait on the logical device to be idle");
}

PhysicalDeviceDetails Device::pickPhysicalDevice(
    vk::Instance vkInstance,
    const std::vector<const char*> deviceExtensions,
    const std::vector<vk::SurfaceKHR>& surfaces) const
{
    auto [result, physDevices] = vkInstance.enumeratePhysicalDevices();
    EXPENGINE_VK_ASSERT(result, "Failed to find a GPU with Vulkan support.");

    /* Ordered map to automatically sort candidates devicess by increasing
     * scores */
    std::multimap<int, PhysicalDeviceDetails> candidates;

    SPDLOG_LOGGER_INFO(logger_, "Automatic selection of a Vulkan compatible GPU:");
    for (const auto& physDevice : physDevices)
    {
        PhysicalDeviceDetails deviceCapabilities
            = ratePhysicalDeviceSuitability(physDevice, surfaces, deviceExtensions);
        candidates.insert(
            std::make_pair(deviceCapabilities.score, deviceCapabilities));
    }

    /* Check if the best candidate is suitable, if so select it. */
    EXPENGINE_ASSERT(
        candidates.rbegin()->first > 0, "Failed to find a suitable GPU");

    PhysicalDeviceDetails pickedPhysDevice = candidates.rbegin()->second;
    /* Store the memory properties of the chosed device */
    pickedPhysDevice.memoryProperties
        = pickedPhysDevice.device.getMemoryProperties();

    SPDLOG_LOGGER_INFO(
        logger_,
        "Selected GPU : {} - {}",
        pickedPhysDevice.properties.deviceID,
        pickedPhysDevice.properties.deviceName);

    return pickedPhysDevice;
}

vk::UniqueDevice Device::createLogicalDevice(
    vk::PhysicalDevice physicalDevice,
    QueueFamilyIndices queueFamilyIndices,
    const std::vector<const char*> deviceExtensions) const
{
    std::vector<vk::DeviceQueueCreateInfo> queueCreatesInfos;
    std::set<uint32_t> uniqueQueueFamiliesIndexes
        = {queueFamilyIndices.graphicsFamily.value(),
           queueFamilyIndices.presentFamily.value()};

    const float queuePriority = 1.0f;
    /* The currently available drivers will only allow you to create a
     * small number of queues for each queue family and you don't
     * really need more than one. That's because you can create all of
     * the command buffers on multiple threads and then
     * submit them all at once on the main thread with a single
     * low-overhead call. */
    for (uint32_t queueFamilyIndex : uniqueQueueFamiliesIndexes)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo {
            .queueFamilyIndex = queueFamilyIndex,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority};
        queueCreatesInfos.push_back(queueCreateInfo);
    }

    /* No features yet */
    vk::PhysicalDeviceFeatures deviceFeatures = {};
    vk::DeviceCreateInfo createInfo = {
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreatesInfos.size()),
        .pQueueCreateInfos = queueCreatesInfos.data(),
        .enabledLayerCount = 0,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    /* https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#extendingvulkan-layers-devicelayerdeprecation
     * EnabledLayerCount and ppEnabledLayerNames fields of
     * VkDeviceCreateInfo are ignored by up-to-date implementations. It is
     * still a good idea to set them anyway to be compatible with older
     * implementations */
    if (vlk::ENABLE_VALIDATION_LAYERS)
    {
        createInfo.enabledLayerCount
            = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = vlk::validationLayers.data();
    }

    auto [result, device] = physicalDevice.createDeviceUnique(createInfo);
    EXPENGINE_VK_ASSERT(result, "Failed to create a logical device");

    return std::move(device);
}

vk::UniqueDescriptorPool Device::createDescriptorPool() const
{
    /* TODO what's the right count ? All the different VK_DESCRIPTOR_TYPE
     * allocated in the ImGui example do not seem necessary here */
    const uint32_t descriptorCount = 100;
    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes {
        {.type = vk::DescriptorType::eCombinedImageSampler,
         .descriptorCount = descriptorCount}};

    vk::DescriptorPoolCreateInfo descriptorPoolInfo {
        .maxSets
        = static_cast<uint32_t>(descriptorCount * descriptorPoolSizes.size()),
        .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
        .pPoolSizes = descriptorPoolSizes.data(),
    };
    auto [result, descriptorPool]
        = logicalDevice_->createDescriptorPoolUnique(descriptorPoolInfo);

    EXPENGINE_VK_ASSERT(result, "Failed to create the descriptor pool");

    return std::move(descriptorPool);
}

uint32_t Device::findMemoryType(
    uint32_t typeFilter,
    vk::MemoryPropertyFlags properties) const
{
    bool found = false;
    auto memProperties = physDevice_.memoryProperties;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i)
            && (memProperties.memoryTypes[i].propertyFlags & properties)
                == properties)
        {
            found = true;
            return i;
        }
    }

    EXPENGINE_ASSERT(found, "Failed to find suitable memory type on GPU");
    return memProperties.memoryTypeCount;
}

} // namespace vlk
} // namespace experim
