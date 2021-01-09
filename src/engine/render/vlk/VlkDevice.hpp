#pragma once

#include <vector>

#include <engine/render/vlk/VlkCapabilities.hpp>
#include <engine/render/vlk/VlkCommandBuffer.hpp>
#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkMemoryAllocator.hpp>
#include <engine/render/vlk/resources/VlkBuffer.hpp>

namespace spdlog {
class logger;
}

namespace expengine {
namespace render {
namespace vlk {

class MemoryAllocator;

class Device {
public:
    Device(
        vk::Instance vkInstance,
        const vk::DispatchLoaderDynamic& dispatchLoader,
        std::shared_ptr<spdlog::logger> logger);
    ~Device();

    /* Public accessors */
    inline const vk::Instance instanceHandle() const { return vkInstance_; }
    inline const vk::Device deviceHandle() const { return logicalDevice_.get(); }
    inline const vk::PhysicalDevice physicalHandle() const
    {
        return physDevice_.device;
    }
    inline const QueueFamilyIndices queueIndices() const
    {
        return physDevice_.queuesIndices;
    }
    inline const vk::Format getDepthFormat() const
    {
        return physDevice_.depthFormat;
    }
    inline const vk::DescriptorPool descriptorPool() const
    {
        return descriptorPool_.get();
    }
    inline const vk::Queue graphicsQueue() const { return graphicsQueue_; }
    inline const vk::Queue presentQueue() const { return presentQueue_; }
    inline const MemoryAllocator& allocator() const { return *memAllocator_; }

    /* Surface support */
    const SwapChainSupportDetails querySwapChainSupport(
        vk::SurfaceKHR& surface) const;
    VkBool32 getSurfaceSupport(vk::SurfaceKHR& surface) const;
    vk::ResultValue<vk::SurfaceCapabilitiesKHR> getSurfaceCapabilities(
        vk::SurfaceKHR& surface) const;

    /* Command buffers */
    const CommandBuffer createTransientCommandBuffer() const;
    const void submitTransientCommandBuffer(CommandBuffer& commandBuffer) const;

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
        const;

    void waitIdle() const;

private:
    /* Instance handle */
    const vk::Instance vkInstance_;

    /* Owned objects */
    PhysicalDeviceDetails physDevice_;
    vk::UniqueDevice logicalDevice_;
    std::unique_ptr<MemoryAllocator> memAllocator_;
    vk::UniqueDescriptorPool descriptorPool_;
    vk::UniqueCommandPool transientCommandPool_;

    /* Handles */
    vk::Queue graphicsQueue_;
    vk::Queue presentQueue_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;

    PhysicalDeviceDetails pickPhysicalDevice(
        vk::Instance vkInstance,
        const std::vector<const char*> deviceExtensions,
        const std::vector<vk::SurfaceKHR>& surfaces) const;

    vk::UniqueDevice createLogicalDevice(
        vk::PhysicalDevice physicalDevice,
        QueueFamilyIndices queueFamilyIndices,
        const std::vector<const char*> deviceExtensions) const;

    vk::UniqueDescriptorPool createDescriptorPool() const;
};
} // namespace vlk
} // namespace render
} // namespace expengine
