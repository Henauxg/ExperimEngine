#include "VlkCapabilities.hpp"

#include <set>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

bool hasInstanceExtensionsSupport(
	std::vector<const char*> requiredExtensions)
{
	/* Acquire Vulkan available extensions */
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
										   nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
										   availableExtensions.data());

	/* Vulkan available extensions */
	SPDLOG_INFO("Available Vulkan Extensions :");
	for (const auto& extension : availableExtensions)
	{
		SPDLOG_INFO("\t {}", extension.extensionName);
	}

	/* Required extensions verification */
	bool extensionsSupported = true;
	SPDLOG_INFO("Required Vulkan Extensions :");
	for (const auto& reqExtension : requiredExtensions)
	{
		bool extFound = false;

		for (const auto& availableExtProps : availableExtensions)
		{
			if (strcmp(reqExtension, availableExtProps.extensionName) == 0)
			{
				extFound = true;
				break;
			}
		}
		if (extFound)
		{
			SPDLOG_INFO("\t {} - Available", reqExtension);
		}
		else
		{
			SPDLOG_INFO("\t {} - Not available", reqExtension);
			extensionsSupported = false;
		}
	}

	return extensionsSupported;
}

/* TODO : Could add logic to explicitly prefer a physical device
 * that supports drawing and presentation in the same queue for improved
 * performance. */
QueueFamilyIndices
findQueueFamilies(vk::PhysicalDevice physDevice,
				  const std::vector<vk::SurfaceKHR>& surfaces)
{
	QueueFamilyIndices queueFamilyIndices;

	std::vector<vk::QueueFamilyProperties> queueFamilies
		= physDevice.getQueueFamilyProperties();

	int currentQueueIndex = 0;
	for (const vk::QueueFamilyProperties& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0
			&& queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			queueFamilyIndices.graphicsFamily = currentQueueIndex;
		}

		/* Ensure that the device can present images to every surface we
		 * created/will create.
		 */
		VkBool32 presentSupport = false;
		for (auto const& surface : surfaces)
		{
			presentSupport = physDevice.getSurfaceSupportKHR(
				currentQueueIndex, surface);
			if (!presentSupport)
				break;
		}

		if (queueFamily.queueCount > 0 && presentSupport)
			queueFamilyIndices.presentFamily = currentQueueIndex;

		if (queueFamilyIndices.isComplete())
			break;

		currentQueueIndex++;
	}

	return queueFamilyIndices;
}

vk::Format findSupportedFormat(vk::PhysicalDevice physicalDevice,
							   const std::vector<vk::Format>& candidates,
							   vk::ImageTiling tiling,
							   vk::FormatFeatureFlags features)
{
	for (vk::Format format : candidates)
	{
		vk::FormatProperties props
			= physicalDevice.getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear
			&& (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == vk::ImageTiling::eOptimal
				 && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	return vk::Format::eUndefined;
}

bool hasPhysDeviceExtensionsSupport(
	vk::PhysicalDevice physDevice,
	const std::vector<const char*> deviceExtensions)
{
	std::vector<vk::ExtensionProperties> availableExtensions
		= physDevice.enumerateDeviceExtensionProperties();

	std::set<std::string> requiredExtensions(deviceExtensions.begin(),
											 deviceExtensions.end());

	SPDLOG_INFO("Required Physical Device Extensions :");
	for (const std::string& extension : requiredExtensions)
	{
		SPDLOG_INFO("-> {}", extension);
	}

	SPDLOG_INFO("Available Physical Device Extensions :");
	for (const auto& extension : availableExtensions)
	{
		SPDLOG_INFO("-> {}", extension.extensionName);
		requiredExtensions.erase(extension.extensionName);
	}

	if (!requiredExtensions.empty())
	{
		SPDLOG_INFO("Unavailable Physical Device Extensions :");
		for (const std::string& extension : requiredExtensions)
		{
			SPDLOG_INFO("-> {}", extension);
		}
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails
queryPhysicalDeviceSwapChainSupport(vk::PhysicalDevice physDevice,
									vk::SurfaceKHR surface)
{
	SwapChainSupportDetails details;
	auto [resultCapabilities, capabilities]
		= physDevice.getSurfaceCapabilitiesKHR(surface);
	auto [resultFormats, formats]
		= physDevice.getSurfaceFormatsKHR(surface);
	auto [resultPresent, presentModes]
		= physDevice.getSurfacePresentModesKHR(surface);
	if (resultCapabilities == vk::Result::eSuccess
		&& resultFormats == vk::Result::eSuccess
		&& resultPresent == vk::Result::eSuccess)
	{
		details.capabilities = capabilities;
		details.formats = formats;
		details.presentModes = presentModes;
	}

	return details;
}

PhysicalDeviceDetails ratePhysicalDeviceSuitability(
	vk::PhysicalDevice physicalDevice,
	const std::vector<vk::SurfaceKHR>& surfaces,
	const std::vector<const char*> deviceExtensions)
{
	PhysicalDeviceDetails deviceDetails;
	deviceDetails.device = physicalDevice;
	deviceDetails.score = 0;
	deviceDetails.properties = physicalDevice.getProperties();
	deviceDetails.features = physicalDevice.getFeatures();

	SPDLOG_INFO("Evaluating Vulkan compatible GPU : {} - {}",
				deviceDetails.properties.deviceID,
				deviceDetails.properties.deviceName);

	/* Discrete GPUs have a significant performance advantage */
	if (deviceDetails.properties.deviceType
		== vk::PhysicalDeviceType::eDiscreteGpu)
	{
		deviceDetails.score += 1000;
	}
	/* Maximum possible size of textures affects graphics quality */
	deviceDetails.score
		+= deviceDetails.properties.limits.maxImageDimension2D;

	/* Application cannot function without the appropriate queues */
	deviceDetails.queuesIndices
		= findQueueFamilies(physicalDevice, surfaces);
	deviceDetails.depthFormat = findSupportedFormat(
		physicalDevice,
		{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint,
		  vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	if (!deviceDetails.queuesIndices.isComplete())
	{
		deviceDetails.score = 0;
		SPDLOG_WARN(" - Incompatible queues on the device");
	}
	else if (!hasPhysDeviceExtensionsSupport(physicalDevice,
											 deviceExtensions))
	{
		deviceDetails.score = 0;
		SPDLOG_WARN(" - Unavailable extensions on the device");
	}
	else if (!deviceDetails.features.samplerAnisotropy)
	{
		/* TODO : add score (+500 ~) if anisotropy is available, else
		 * memorize somewhere that the chosen device does not support
		 * anisotropy and don't ask for it in ie: samplers... */
		deviceDetails.score = 0;
		SPDLOG_WARN(" - Anisotropy feature not available on the device");
	}
	else if (deviceDetails.depthFormat == vk::Format::eUndefined)
	{
		deviceDetails.score = 0;
		SPDLOG_WARN(" - No suitable depth format available on the device");
	}
	else
	{
		bool swapChainAdequate = false;
		for (auto const& surface : surfaces)
		{
			SwapChainSupportDetails swapChainSupport
				= queryPhysicalDeviceSwapChainSupport(physicalDevice,
													  surface);
			// TODO More work on formats and present mode. See main.cpp of
			// Imgui SDL+Vulkan example. Note : in this main.cpp it is used
			// as an application pov once the gpu has already been
			// selected.
			swapChainAdequate = !swapChainSupport.formats.empty()
				&& !swapChainSupport.presentModes.empty();
			if (!swapChainAdequate)
			{
				deviceDetails.score = 0;
				SPDLOG_WARN(" - Swapchain support not suitable");
				break;
			}
		}
	}

	SPDLOG_INFO("{} - Rated score : {}",
				deviceDetails.properties.deviceName, deviceDetails.score);

	return deviceDetails;
}

} // namespace vlk
} // namespace render
} // namespace expengine
