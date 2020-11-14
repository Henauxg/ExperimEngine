#pragma once

#include <vector>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkCapabilities.hpp>
#include <engine/render/vlk/VlkDebug.hpp>
#include <engine/render/vlk/VlkInclude.hpp>
#include <engine/render/vlk/resources/VlkBuffer.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Device {
public:
	Device(vk::Instance vkInstance,
		   std::shared_ptr<spdlog::logger> logger);
	~Device();

	/* Accessors */
	inline const vk::Instance instanceHandle() const
	{
		return vkInstance_;
	}
	inline const vk::Device deviceHandle() const
	{
		return logicalDevice_.get();
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

	inline const SwapChainSupportDetails
	querySwapChainSupport(vk::SurfaceKHR& surface) const
	{
		return queryPhysicalDeviceSwapChainSupport(physDevice_.device,
												   surface);
	}

	inline VkBool32 getSurfaceSupport(vk::SurfaceKHR& surface) const
	{
		return physDevice_.device.getSurfaceSupportKHR(
			queueIndices().presentFamily.value(), surface);
	}

	const vk::CommandBuffer createTransientCommandBuffer() const;

	const void
	submitTransientCommandBuffer(vk::CommandBuffer commandBuffer) const;

	std::unique_ptr<vlk::Buffer>
	createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags,
				 vk::MemoryPropertyFlags memPropertyFlags,
				 void const* data) const;

	uint32_t findMemoryType(uint32_t typeFilter,
							vk::MemoryPropertyFlags properties) const;

	void waitIdle() const;

private:
	/* Instance reference */
	const vk::Instance& vkInstance_;

	PhysicalDeviceDetails physDevice_;
	vk::UniqueDevice logicalDevice_;
	vk::UniqueDescriptorPool descriptorPool_;
	vk::UniqueCommandPool transientCommandPool_;
	vk::Queue graphicsQueue_;
	vk::Queue presentQueue_;

	/* Logging */
	std::shared_ptr<spdlog::logger> logger_;

	PhysicalDeviceDetails
	pickPhysicalDevice(vk::Instance vkInstance,
					   const std::vector<const char*> deviceExtensions,
					   const std::vector<vk::SurfaceKHR>& surfaces);

	vk::UniqueDevice
	createLogicalDevice(vk::PhysicalDevice physicalDevice,
						QueueFamilyIndices queueFamilyIndices,
						const std::vector<const char*> deviceExtensions);

	vk::UniqueDescriptorPool createDescriptorPool() const;
};
} // namespace vlk
} // namespace render
} // namespace expengine
