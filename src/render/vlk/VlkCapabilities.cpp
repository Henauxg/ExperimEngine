#include "VlkCapabilities.hpp"

#include "vlk/VlkInclude.hpp"
#include <iostream>

namespace expengine {
namespace render {
namespace vlk {

/* TODO : log */
bool hasInstanceExtensionsSupport(std::vector<const char*> requiredExtensions)
{
	/* Acquire Vulkan available extensions */
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	/* Vulkan available extensions */
	std::cout << "Available Vulkan Extensions : " << std::endl;
	for (const auto& extension : availableExtensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	/* Required extensions verification */
	bool extensionsSupported = true;
	std::cout << "Required Extensions : " << std::endl;
	for (const auto& reqExtension : requiredExtensions) {
		std::cout << "\t" << reqExtension;
		bool extFound = false;

		for (const auto& availableExtProps : availableExtensions) {
			if (strcmp(reqExtension, availableExtProps.extensionName) == 0) {
				extFound = true;
				break;
			}
		}
		if (extFound) {
			std::cout << " - Available" << std::endl;
		} else {
			std::cout << " - Not available" << std::endl;
			extensionsSupported = false;
		}
	}

	return extensionsSupported;
}
} // namespace vlk
} // namespace render
} // namespace expengine