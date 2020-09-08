#pragma once

#include <iostream>
#include <vector>

#include <engine/render/vlk/VlkInclude.hpp>

#if defined(__ANDROID__)
/* TODO Update this. */
#define ASSERT_VK_RESULT(f, message)                                      \
	{                                                                     \
		VkResult res = (f);                                               \
		if (res != VK_SUCCESS)                                            \
		{                                                                 \
			LOGE("Fatal : VkResult is \" %s \" in %s at line %d",         \
				 vlk::vkResultToString(res).c_str(), __FILE__, __LINE__); \
			abort();                                                      \
		}                                                                 \
	}
#define ASSERT_RESULT(f, message)                                         \
	{                                                                     \
		bool expr = (f);                                                  \
		if (!expr)                                                        \
		{                                                                 \
			LOGE("Fatal : assert failed in %s at line %d", __FILE__,      \
				 __LINE__);                                               \
			abort();                                                      \
		}                                                                 \
	}
#else
#define ASSERT_VK_RESULT(f, message)                                      \
	{                                                                     \
		vk::Result res = (f);                                             \
		if (res != vk::Result::eSuccess)                                  \
		{                                                                 \
			std::cout << "Fatal : VkResult is \""                         \
					  << vlk::vkResultToString(res) << "\" in "           \
					  << __FILE__ << " at line " << __LINE__              \
					  << ", message : " << message << std::endl;          \
			abort();                                                      \
		}                                                                 \
	}
#define ASSERT_RESULT(f, message)                                         \
	{                                                                     \
		bool expr = (f);                                                  \
		if (!expr)                                                        \
		{                                                                 \
			std::cout << "Fatal : assert failed in " << __FILE__          \
					  << " at line " << __LINE__                          \
					  << ", message : " << message << std::endl;          \
			abort();                                                      \
		}                                                                 \
	}
#endif

namespace expengine {
namespace render {
namespace vlk {

/* vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose */
/* vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo */
static vk::DebugUtilsMessageSeverityFlagsEXT debugCallbackMessageSeverity
	= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
	| vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

static vk::DebugUtilsMessageTypeFlagsEXT debugCallbackMessageType
	= vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
	| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
	| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

const std::vector<const char*> validationLayers
	= { "VK_LAYER_KHRONOS_validation" };

bool hasValidationLayerSupport(
	const std::vector<const char*> validationLayers);

vk::DebugUtilsMessengerCreateInfoEXT
getDebugUtilsCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
vk::DebugUtilsMessengerEXT
createDebugUtilsMessengerEXT(vk::Instance instance);
void destroyDebugUtilsMessengerEXT(
	vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger);

std::string vkResultToString(vk::Result errorCode);

} // namespace vlk
} // namespace render
} // namespace expengine
