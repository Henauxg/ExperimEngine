#include "VlkDevice.hpp"

#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include <engine/render/Window.hpp>
#include <engine/render/vlk/VlkDebug.hpp>

namespace expengine {
namespace render {
namespace vlk {

const std::vector<const char*> DEVICE_EXTENSIONS
	= { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

Device::Device(vk::Instance vkInstance,
			   std::shared_ptr<spdlog::logger> logger)
	: logger_(logger)
{
	/* Create a temporary dummy window/surface to get information
	 * about surface compatibility between the Vulkan devices
	 * selected/created and the surfaces that will be in use during the
	 * application runtime. */
	auto dummyWindow = std::make_unique<render::Window>();
	auto [surfaceCreated, dummySurface]
		= dummyWindow->createVkSurface(vkInstance);
	EXPENGINE_ASSERT(surfaceCreated, "Failed to create a VkSurface");
	auto windowSurface_ = vk::UniqueSurfaceKHR(dummySurface, vkInstance);

	/* Devices */
	physDevice_ = pickPhysicalDevice(
		vkInstance, DEVICE_EXTENSIONS,
		std::vector<vk::SurfaceKHR> { *windowSurface_ });
	logicalDevice_ = createLogicalDevice(
		physDevice_.device, physDevice_.queuesIndices, DEVICE_EXTENSIONS);

	/* Queues handles */
	graphicsQueue_ = logicalDevice_->getQueue(
		physDevice_.queuesIndices.graphicsFamily.value(), 0);
	presentQueue_ = logicalDevice_->getQueue(
		physDevice_.queuesIndices.presentFamily.value(), 0);

	/* Descriptor pool */
	descriptorPool_ = createDescriptorPool();

	/* Command pool for short lived buffers */
	auto cmdPoolResult = logicalDevice_->createCommandPoolUnique(
		{ .flags = vk::CommandPoolCreateFlagBits::eTransient,
		  .queueFamilyIndex
		  = physDevice_.queuesIndices.graphicsFamily.value() });
	EXPENGINE_VK_ASSERT(
		cmdPoolResult.result,
		"Failed to create the device transient command pool");
	transientCommandPool_ = std::move(cmdPoolResult.value);
}

inline const vk::CommandBuffer Device::getTransientCommandBuffer() const
{
	auto [allocResult, cmdBuffers]
		= logicalDevice_->allocateCommandBuffers(
			{ .commandPool = *transientCommandPool_,
			  .commandBufferCount = 1 });
	EXPENGINE_VK_ASSERT(allocResult,
						"Failed to allocate a command buffer");

	auto commandBuffer = cmdBuffers.front();
	auto beginResult = commandBuffer.begin(
		{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	EXPENGINE_VK_ASSERT(beginResult,
						"Failed to begin on a command buffer");

	return commandBuffer;
}

const void
Device::submitTransientCommandBuffer(vk::CommandBuffer commandBuffer) const
{
	auto endResult = commandBuffer.end();
	EXPENGINE_VK_ASSERT(endResult, "Failed to end on a command buffer");

	vk::SubmitInfo submitInfo { .commandBufferCount = 1,
								.pCommandBuffers = &commandBuffer };
	/* TODO : Could implement fences for transient buffers ? */
	graphicsQueue_.submit(submitInfo, nullptr);
	graphicsQueue_.waitIdle();

	logicalDevice_->freeCommandBuffers(*transientCommandPool_,
									   commandBuffer);
}

void Device::waitIdle() const { logicalDevice_->waitIdle(); }

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

vk::UniqueDescriptorPool Device::createDescriptorPool() const
{
	/* TODO what's the right count ? All the different VK_DESCRIPTOR_TYPE
	 * allocated in the ImGui example do not seem necessary here */
	const uint32_t descriptorCount = 100;
	std::vector<vk::DescriptorPoolSize> descriptorPoolSizes {
		{ .type = vk::DescriptorType::eCombinedImageSampler,
		  .descriptorCount = descriptorCount }
	};

	vk::DescriptorPoolCreateInfo descriptorPoolInfo {
		.maxSets = static_cast<uint32_t>(descriptorCount
										 * descriptorPoolSizes.size()),
		.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()),
		.pPoolSizes = descriptorPoolSizes.data(),
	};
	auto [result, descriptorPool]
		= logicalDevice_->createDescriptorPoolUnique(descriptorPoolInfo);

	EXPENGINE_VK_ASSERT(result, "Failed to create the descriptor pool");

	return std::move(descriptorPool);
}

} // namespace vlk
} // namespace render
} // namespace expengine
