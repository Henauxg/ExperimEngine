#include "Renderer.hpp"

#include "ExperimEngineConfig.h"
#include "vlk/VlkCapabilities.hpp"
#include "vlk/VlkDebug.hpp"
#include <stdexcept>

namespace expengine {
namespace render {

Renderer::Renderer(const char* appName, const Window& window,
				   EngineParameters& engineParams)
	: window_(window)
	, engineParams_(engineParams)
{
	vkInstance_ = createVulkanInstance(appName, window);
	vkDebugMessenger_ = setupDebugMessenger(*vkInstance_, vlk::ENABLE_VALIDATION_LAYERS);
	vkSurface_ = vk::UniqueSurfaceKHR(window_.createSurface(*vkInstance_), *vkInstance_);
	// vkPhysicalDevice_ = pickPhysicalDevice(vkInstance_, vkSurface_, deviceExtensions);
	// auto device = testDevice.createDevice(vk::DeviceCreateInfo());

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

vk::UniqueInstance Renderer::createVulkanInstance(const char* appName,
												  const Window& window) const
{
	/* Check layer support */
	ASSERT_RESULT(!vlk::ENABLE_VALIDATION_LAYERS
					  || vlk::hasValidationLayerSupport(vlk::validationLayers),
				  "Validation layer(s) requested, but not available.");

	/* Acquire all the required extensions */
	std::vector<const char*> extensions = window.getRequiredVkExtensions();
	if (vlk::ENABLE_VALIDATION_LAYERS) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	/* Check extension support */
	ASSERT_RESULT(vlk::hasInstanceExtensionsSupport(extensions),
				  "Vulkan extension(s) not supported.");

	/* Instance structs */
	vk::ApplicationInfo appInfo(
		appName,
		VK_MAKE_VERSION(ExperimEngine_VERSION_MAJOR, ExperimEngine_VERSION_MINOR,
						ExperimEngine_VERSION_PATCH),
		"No Engine", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_0);

	vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlags(), &appInfo, 0, nullptr,
									  static_cast<uint32_t>(extensions.size()),
									  extensions.data());

	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (vlk::ENABLE_VALIDATION_LAYERS) {
		createInfo.enabledLayerCount
			= static_cast<uint32_t>(vlk::validationLayers.size());
		createInfo.ppEnabledLayerNames = vlk::validationLayers.data();
		createInfo.pNext
			= (VkDebugUtilsMessengerCreateInfoEXT*) (&vlk::getDebugUtilsCreateInfo(
				debugCreateInfo));
	}

	auto [result, instance] = vk::createInstance(createInfo);
	ASSERT_VK_RESULT(result, "Failed to create Vulkan instance.");

	return vk::UniqueInstance(instance);
}

vk::DebugUtilsMessengerEXT
Renderer::setupDebugMessenger(vk::Instance instance, bool enableValidationLayers) const
{
	if (!enableValidationLayers) {
		return nullptr;
	}

	return vlk::createDebugUtilsMessengerEXT(instance);
}

} // namespace render
} // namespace expengine
