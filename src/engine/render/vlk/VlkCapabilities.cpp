#include "VlkCapabilities.hpp"

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

/* TODO : log */
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
} // namespace vlk
} // namespace render
} // namespace expengine