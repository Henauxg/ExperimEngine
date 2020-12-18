#pragma once

#include <vector>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

#if defined(__ANDROID__)
/* TODO */
#else

#define EXPENGINE_VK_ASSERT(f, ...)                                                 \
    do                                                                              \
    {                                                                               \
        vk::Result __res__ = (f);                                                   \
        if (__res__ != vk::Result::eSuccess)                                        \
        {                                                                           \
            SPDLOG_ERROR(                                                           \
                "Fatal : assert failed. Program will abort. VkResult"               \
                " is \"{}\"",                                                       \
                vk::to_string(__res__));                                            \
            SPDLOG_ERROR(__VA_ARGS__);                                              \
            abort();                                                                \
        }                                                                           \
    } while (0)

#endif

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

const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

bool hasValidationLayerSupport(const std::vector<const char*> validationLayers);
vk::DebugUtilsMessengerCreateInfoEXT getDebugUtilsCreateInfo(
    vk::DebugUtilsMessengerCreateInfoEXT& createInfo,
    spdlog::logger* logger);
vk::DebugUtilsMessengerEXT createDebugUtilsMessengerEXT(
    vk::Instance instance,
    spdlog::logger* logger);
void destroyDebugUtilsMessengerEXT(
    vk::Instance instance,
    vk::DebugUtilsMessengerEXT debugMessenger);

} // namespace vlk
} // namespace render
} // namespace expengine
