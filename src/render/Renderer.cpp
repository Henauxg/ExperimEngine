#include "Renderer.hpp"

#include "ExperimEngineConfig.h"
#include "vlk/VlkCapabilities.hpp"
#include "vlk/VlkDebug.hpp"
#include <stdexcept>

namespace expengine {
namespace render {

Renderer::Renderer(const char* appName, const Window& window, EngineParameters& engineParams)
	: window_(window)
	, engineParams_(engineParams)
{
	vkInstance_ = createVulkanInstance(appName, window);
	vkDebugMessenger_ = setupDebugMessenger(*vkInstance_, vlk::ENABLE_VALIDATION_LAYERS);
	// vkSurface_ = window_.createSurface(*vkInstance_);
	vkSurface_ = vk::UniqueSurfaceKHR(window_.createSurface(*vkInstance_), *vkInstance_);
	/* TODO Implement */
}

Renderer::~Renderer()
{ /* TODO Implement */

	if (vlk::ENABLE_VALIDATION_LAYERS) {
		vlk::destroyDebugUtilsMessengerEXT(*vkInstance_, vkDebugMessenger_);
	}
}

void Renderer::render()
{ /* TODO Implement */
}

void Renderer::rendererWaitIdle()
{ /* TODO Implement */
}

vk::UniqueInstance Renderer::createVulkanInstance(const char* appName, const Window& window) const
{
	/* Check layer support */
	if (vlk::ENABLE_VALIDATION_LAYERS && !vlk::hasValidationLayerSupport(vlk::validationLayers))
		throw std::runtime_error("Validation layer(s) requested, but not available.");

	/* Acquire all the required extensions */
	std::vector<const char*> extensions = window.getRequiredVkExtensions();
	if (vlk::ENABLE_VALIDATION_LAYERS) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	/* Check extension support */
	if (!vlk::hasInstanceExtensionsSupport(extensions))
		throw std::runtime_error("Vulkan extension(s) not supported.");

	/* Instance structs */
	vk::ApplicationInfo appInfo(appName,
								VK_MAKE_VERSION(ExperimEngine_VERSION_MAJOR,
												ExperimEngine_VERSION_MINOR,
												ExperimEngine_VERSION_PATCH),
								"No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

	vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(), &appInfo, 0, nullptr,
									  static_cast<uint32_t>(extensions.size()), extensions.data());

	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (vlk::ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(vlk::validationLayers.size());
		createInfo.ppEnabledLayerNames = vlk::validationLayers.data();
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) (&vlk::getDebugUtilsCreateInfo(
			debugCreateInfo));
	}

	vk::UniqueInstance createdVkInstance = vk::createInstanceUnique(createInfo);

	return createdVkInstance;
}

vk::DebugUtilsMessengerEXT Renderer::setupDebugMessenger(vk::Instance instance,
														 bool enableValidationLayers) const
{
	if (!enableValidationLayers)
		return nullptr;

	return vlk::createDebugUtilsMessengerEXT(instance);
}

} // namespace render
} // namespace expengine
