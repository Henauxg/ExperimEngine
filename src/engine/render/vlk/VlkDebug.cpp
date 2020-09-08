#include "VlkDebug.hpp"

#include <cstdint>

#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

/* TODO : log */
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			  VkDebugUtilsMessageSeverityFlagsEXT messageType,
			  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			  void* pUserData)
{
	std::cerr << "Validation layer : " << pCallbackData->pMessage
			  << std::endl;

	return VK_FALSE;
}

vk::DebugUtilsMessengerCreateInfoEXT
getDebugUtilsCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
		{}, vlk::debugCallbackMessageSeverity,
		vlk::debugCallbackMessageType, debugCallback);
	return createInfo;
}

vk::DebugUtilsMessengerEXT
createDebugUtilsMessengerEXT(vk::Instance instance)
{
	/* https://github.com/dokipen3d/vulkanHppMinimalExample/blob/master/main.cpp
	 */
	/*auto debugMessenger =
	vkInstance_->createDebugUtilsMessengerEXTUnique( createInfo, nullptr,
	vk::DispatchLoaderDynamic { *vkInstance_ });*/
	/* https://github.com/KhronosGroup/Vulkan-Hpp/issues/443 */
	/* 	vk::UniqueHandle<vk::DebugUtilsMessengerEXT,
	   vk::DispatchLoaderStatic> vkDebugMessenger_ =
	   vkInstance_->createDebugUtilsMessengerEXTUnique(createInfo); */

	vk::DebugUtilsMessengerCreateInfoEXT createInfo;
	auto [result, vkDebugMessenger]
		= instance.createDebugUtilsMessengerEXT(
			getDebugUtilsCreateInfo(createInfo), nullptr,
			vk::DispatchLoaderDynamic { instance,
										&vkGetInstanceProcAddr });

	ASSERT_VK_RESULT(result,
					 "Debug Utils Messenger could not be created.");

	return vkDebugMessenger;
}

void destroyDebugUtilsMessengerEXT(
	vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger)
{
	instance.destroyDebugUtilsMessengerEXT(
		debugMessenger, nullptr,
		vk::DispatchLoaderDynamic { instance, &vkGetInstanceProcAddr });
}

/* TODO : log */
bool hasValidationLayerSupport(
	const std::vector<const char*> validationLayers)
{
	uint32_t layerCount;
	/* Get number of layers */
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	/* Get the available layers */
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount,
									   availableLayers.data());

	/* Verify that the layers we want are available */
	std::cout << "Requested Layers : " << std::endl;
	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
		{
			std::cout << layerName << " - Not available" << std::endl;
			return false;
		}
		else
		{
			std::cout << layerName << " - Available" << std::endl;
		}
	}
	return true;
}

std::string vkResultToString(vk::Result errorCode)
{
	switch (errorCode)
	{
#define STR(r)                                                            \
	case r:                                                               \
		return #r
		STR(vk::Result::eNotReady);
		STR(vk::Result::eTimeout);
		STR(vk::Result::eEventSet);
		STR(vk::Result::eEventReset);
		STR(vk::Result::eIncomplete);
		STR(vk::Result::eErrorOutOfHostMemory);
		STR(vk::Result::eErrorOutOfDeviceMemory);
		STR(vk::Result::eErrorInitializationFailed);
		STR(vk::Result::eErrorDeviceLost);
		STR(vk::Result::eErrorMemoryMapFailed);
		STR(vk::Result::eErrorLayerNotPresent);
		STR(vk::Result::eErrorExtensionNotPresent);
		STR(vk::Result::eErrorFeatureNotPresent);
		STR(vk::Result::eErrorIncompatibleDriver);
		STR(vk::Result::eErrorTooManyObjects);
		STR(vk::Result::eErrorFormatNotSupported);
		STR(vk::Result::eErrorFragmentedPool);
		STR(vk::Result::eErrorOutOfPoolMemory);
		STR(vk::Result::eErrorInvalidExternalHandle);
		STR(vk::Result::eErrorSurfaceLostKHR);
		STR(vk::Result::eErrorNativeWindowInUseKHR);
		STR(vk::Result::eSuboptimalKHR);
		STR(vk::Result::eErrorOutOfDateKHR);
		STR(vk::Result::eErrorIncompatibleDisplayKHR);
		STR(vk::Result::eErrorValidationFailedEXT);
		STR(vk::Result::eErrorInvalidShaderNV);
		STR(vk::Result::eErrorFragmentationEXT);
		STR(vk::Result::eErrorNotPermittedEXT);
#undef STR
	default:
		return "UNKNOWN_ERROR";
	}
}

} // namespace vlk
} // namespace render
} // namespace expengine