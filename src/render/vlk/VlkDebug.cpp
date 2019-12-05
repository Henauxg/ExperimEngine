#include "VlkDebug.hpp"

#include "vlk/VlkInclude.hpp"
#include <cstdint>
#include <iostream>

namespace expengine {
namespace render {
namespace vlk {

/* TODO : log */
bool hasValidationLayerSupport(const std::vector<const char*> validationLayers)
{
	uint32_t layerCount;
	/* Get number of layers */
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	/* Get the available layers */
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	/* Verify that the layers we want are available */
	std::cout << "Requested Layers : " << std::endl;
	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			std::cout << layerName << " - Not available" << std::endl;
			return false;
		} else {
			std::cout << layerName << " - Available" << std::endl;
		}
	}
	return true;
}

} // namespace vlk
} // namespace render
} // namespace expengine