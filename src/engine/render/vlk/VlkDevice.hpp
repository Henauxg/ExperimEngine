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
	/**  @brief Takes 1 or more surface for compatibility between the
	 * Vulkan objects created afterwards and the surfaces in use. */
	Device(vk::Instance vkInstance,
		   const std::vector<vk::SurfaceKHR>& surfaces,
		   std::shared_ptr<spdlog::logger> logger);

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	inline const vk::Device getDeviceHande() const
	{
		return logicalDevice_.get();
	}
	inline const QueueFamilyIndices getQueueIndices() const
	{
		return physDevice_.queuesIndices;
	}
	inline const vk::Format getDepthFormat() const
	{
		return physDevice_.depthFormat;
	}

private:
	PhysicalDeviceDetails physDevice_;
	vk::UniqueDevice logicalDevice_;

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
};
} // namespace vlk
} // namespace render
} // namespace expengine
