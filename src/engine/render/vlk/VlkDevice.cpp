#include "VlkDevice.hpp"

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include <engine/render/vlk/VlkDebug.hpp>

namespace expengine {
namespace render {
namespace vlk {

const std::vector<const char*> DEVICE_EXTENSIONS
	= { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

Device::Device(vk::Instance vkInstance,
			   const std::vector<vk::SurfaceKHR>& surfaces,
			   std::shared_ptr<spdlog::logger> logger)
	: logger_(logger)
{
	EXPENGINE_ASSERT(surfaces.size() > 0,
					 "The Device creation needs at least 1 surface to "
					 "decide on the physical device.");
	physDevice_
		= pickPhysicalDevice(vkInstance, DEVICE_EXTENSIONS, surfaces);
	logicalDevice_ = createLogicalDevice(
		physDevice_.device, physDevice_.queuesIndices, DEVICE_EXTENSIONS);
}

PhysicalDeviceDetails
Device::pickPhysicalDevice(vk::Instance vkInstance,
						   const std::vector<const char*> deviceExtensions,
						   const std::vector<vk::SurfaceKHR>& surfaces)
{
	auto [result, physDevices] = vkInstance.enumeratePhysicalDevices();
	EXPENGINE_VK_ASSERT(result,
						"Failed to find a GPU with Vulkan support.");

	/* Ordered map to automatically sort candidates devicess by increasing
	 * scores */
	std::multimap<int, PhysicalDeviceDetails> candidates;

	SPDLOG_LOGGER_INFO(logger_,
					   "Automatic selection of a Vulkan compatible GPU:");
	for (const auto& physDevice : physDevices)
	{
		PhysicalDeviceDetails deviceCapabilities
			= ratePhysicalDeviceSuitability(physDevice, surfaces,
											deviceExtensions);
		candidates.insert(
			std::make_pair(deviceCapabilities.score, deviceCapabilities));
	}

	/* Check if the best candidate is suitable, if so select it. */
	EXPENGINE_ASSERT(candidates.rbegin()->first > 0,
					 "Failed to find a suitable GPU");

	PhysicalDeviceDetails pickedPhysDevice = candidates.rbegin()->second;

	SPDLOG_LOGGER_INFO(logger_, "Selected GPU : {} - {}",
					   pickedPhysDevice.properties.deviceID,
					   pickedPhysDevice.properties.deviceName);

	return pickedPhysDevice;
}

vk::UniqueDevice Device::createLogicalDevice(
	vk::PhysicalDevice physicalDevice,
	QueueFamilyIndices queueFamilyIndices,
	const std::vector<const char*> deviceExtensions)
{
	std::vector<vk::DeviceQueueCreateInfo> queueCreatesInfos;
	std::set<uint32_t> uniqueQueueFamiliesIndexes
		= { queueFamilyIndices.graphicsFamily.value(),
			queueFamilyIndices.presentFamily.value() };

	const float queuePriority = 1.0f;
	/* The currently available drivers will only allow you to create a
	 * small number of queues for each queue family and you don't
	 * really need more than one. That's because you can create all of
	 * the command buffers on multiple threads and then
	 * submit them all at once on the main thread with a single
	 * low-overhead call. */
	for (uint32_t queueFamilyIndex : uniqueQueueFamiliesIndexes)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo { .queueFamilyIndex
													= queueFamilyIndex,
													.queueCount = 1,
													.pQueuePriorities
													= &queuePriority };
		queueCreatesInfos.push_back(queueCreateInfo);
	}

	/* No features yet */
	vk::PhysicalDeviceFeatures deviceFeatures = {};
	vk::DeviceCreateInfo createInfo = {
		.queueCreateInfoCount
		= static_cast<uint32_t>(queueCreatesInfos.size()),
		.pQueueCreateInfos = queueCreatesInfos.data(),
		.enabledLayerCount = 0,
		.enabledExtensionCount
		= static_cast<uint32_t>(deviceExtensions.size()),
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

} // namespace vlk
} // namespace render
} // namespace expengine