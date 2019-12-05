#pragma once

#include "vlk/VlkInclude.hpp"
#include <vector>

namespace expengine {
namespace render {
namespace vlk {

static vk::DebugUtilsMessageSeverityFlagsEXT debugCallbackMessageSeverity
	= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
	| vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
	| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
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

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

bool hasValidationLayerSupport(const std::vector<const char*> validationLayers);
vk::DebugUtilsMessengerEXT createDebugUtilsMessengerEXT(vk::Instance instance);
void destroyDebugUtilsMessengerEXT(vk::Instance instance,
								   vk::DebugUtilsMessengerEXT debugMessenger);

} // namespace vlk
} // namespace render
} // namespace expengine
