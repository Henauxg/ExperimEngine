#pragma once

#include <optional>
#include <vector>

#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return (graphicsFamily.has_value() && presentFamily.has_value());
	}
};

struct SwapChainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};

struct PhysicalDeviceDetails {
	uint32_t score;
	vk::PhysicalDevice device;
	vk::PhysicalDeviceFeatures features;
	vk::PhysicalDeviceProperties properties;
	QueueFamilyIndices queuesIndices;
	vk::Format depthFormat;
};

bool hasInstanceExtensionsSupport(std::vector<const char*> extensions);
PhysicalDeviceDetails ratePhysicalDeviceSuitability(
	vk::PhysicalDevice physDevice,
	const std::vector<vk::SurfaceKHR>& surfaces,
	const std::vector<const char*> deviceExtensions);

bool hasPhysDeviceExtensionsSupport(
	vk::PhysicalDevice physDevice,
	const std::vector<const char*> deviceExtensions);

SwapChainSupportDetails
queryPhysicalDeviceSwapChainSupport(vk::PhysicalDevice physDevice,
									vk::SurfaceKHR surface);

QueueFamilyIndices
findQueueFamilies(vk::PhysicalDevice physDevice,
				  const std::vector<vk::SurfaceKHR>& surfaces);

vk::Format findSupportedFormat(vk::PhysicalDevice physicalDevice,
							   const std::vector<vk::Format>& candidates,
							   vk::ImageTiling tiling,
							   vk::FormatFeatureFlags features);

} // namespace vlk
} // namespace render
} // namespace expengine