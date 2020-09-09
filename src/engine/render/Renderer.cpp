#include "Renderer.hpp"

#include <stdexcept>

#include <ExperimEngineConfig.h>
#include <engine/render/vlk/VlkCapabilities.hpp>
#include <engine/render/vlk/VlkDebug.hpp>

namespace expengine {
namespace render {

Renderer::Renderer(const char* appName, const Window& window,
				   EngineParameters& engineParams)
	: window_(window)
	, engineParams_(engineParams)
	, logger_(spdlog::get(LOGGER_NAME))
{
	vkInstance_ = createVulkanInstance(appName, window);

	vkDebugMessenger_
		= setupDebugMessenger(*vkInstance_, vlk::ENABLE_VALIDATION_LAYERS);
	vkSurface_ = vk::UniqueSurfaceKHR(window_.createSurface(*vkInstance_),
									  *vkInstance_);

	vlkDevice_ = std::make_unique<vlk::Device>(
		*vkInstance_, std::vector<vk::SurfaceKHR> { *vkSurface_ },
		logger_);

	vkDescriptorPool_ = createDescriptorPool(vlkDevice_->getDeviceHande());
}

Renderer::~Renderer()
{
	/* TODO Implement */
	if (vlk::ENABLE_VALIDATION_LAYERS)
	{
		vlk::destroyDebugUtilsMessengerEXT(*vkInstance_,
										   vkDebugMessenger_);
	}
}

void Renderer::render()
{ /* TODO Implement */
}

void Renderer::rendererWaitIdle()
{ /* TODO Implement */
}

vk::UniqueInstance
Renderer::createVulkanInstance(const char* appName,
							   const Window& window) const
{
	/* Check layer support */
	EXPENGINE_ASSERT(
		!vlk::ENABLE_VALIDATION_LAYERS
			|| vlk::hasValidationLayerSupport(vlk::validationLayers),
		"Validation layer(s) requested, but not available.");

	/* Acquire all the required extensions */
	std::vector<const char*> extensions = window.getRequiredVkExtensions();
	if (vlk::ENABLE_VALIDATION_LAYERS)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	/* Check extension support */
	EXPENGINE_ASSERT(vlk::hasInstanceExtensionsSupport(extensions),
					 "Vulkan extension(s) not supported.");

	/* Instance structs */
	vk::ApplicationInfo applicationInfo {
		.pApplicationName = appName,
		.applicationVersion = VK_MAKE_VERSION(ExperimEngine_VERSION_MAJOR,
											  ExperimEngine_VERSION_MINOR,
											  ExperimEngine_VERSION_PATCH),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_0
	};

	vk::InstanceCreateInfo createInfo {
		.pApplicationInfo = &applicationInfo,
		.enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
		.ppEnabledExtensionNames = extensions.data()
	};

	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (vlk::ENABLE_VALIDATION_LAYERS)
	{
		createInfo.enabledLayerCount
			= static_cast<uint32_t>(vlk::validationLayers.size());
		createInfo.ppEnabledLayerNames = vlk::validationLayers.data();
		/* Give logger access to Debug Messenger */
		debugCreateInfo = vlk::getDebugUtilsCreateInfo(debugCreateInfo);
		debugCreateInfo.pUserData = logger_.get();
		createInfo.pNext = &debugCreateInfo;
	}

	auto [result, instance] = vk::createInstance(createInfo);
	EXPENGINE_VK_ASSERT(result, "Failed to create Vulkan instance.");

	return vk::UniqueInstance(instance);
}

vk::DebugUtilsMessengerEXT
Renderer::setupDebugMessenger(vk::Instance instance,
							  bool enableValidationLayers) const
{
	if (!enableValidationLayers)
	{
		return nullptr;
	}

	return vlk::createDebugUtilsMessengerEXT(instance);
}

vk::UniqueDescriptorPool
Renderer::createDescriptorPool(vk::Device device) const
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
		= vlkDevice_->getDeviceHande().createDescriptorPool(
			descriptorPoolInfo);

	EXPENGINE_VK_ASSERT(result, "Failed to create the descriptor pool");

	return vk::UniqueDescriptorPool(descriptorPool);
}

} // namespace render
} // namespace expengine
