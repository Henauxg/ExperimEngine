#pragma once
#pragma once

#include <vector>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkCapabilities.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Device {
public:
	Device(vk::Instance vkInstance,
		   std::shared_ptr<spdlog::logger> logger);

	/* Accessors */
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

	const vk::CommandBuffer getTransientCommandBuffer() const;
	const void
	submitTransientCommandBuffer(vk::CommandBuffer commandBuffer) const;
	void waitIdle() const;

private:
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

	vk::UniqueDescriptorPool Device::createDescriptorPool() const;
};
} // namespace vlk
} // namespace render
} // namespace expengine
