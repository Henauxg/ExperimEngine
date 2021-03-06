#include "VlkDebug.hpp"

#include <cstdint>

#include <engine/render/vlk/VlkInclude.hpp>

namespace experim {
namespace vlk {

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageSeverityFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    auto logger = (spdlog::logger*) pUserData;
    spdlog::level::level_enum level = spdlog::level::err;
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        level = spdlog::level::trace;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        level = spdlog::level::info;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        level = spdlog::level::warn;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
    default:
        break;
    }

    SPDLOG_LOGGER_CALL(
        logger, level, "Validation layer : {}", pCallbackData->pMessage);

    return VK_FALSE;
}

vk::DebugUtilsMessengerCreateInfoEXT getDebugUtilsCreateInfo(
    vk::DebugUtilsMessengerCreateInfoEXT& createInfo,
    spdlog::logger* logger)
{
    createInfo = vk::DebugUtilsMessengerCreateInfoEXT {
        .messageSeverity = vlk::debugCallbackMessageSeverity,
        .messageType = vlk::debugCallbackMessageType,
        .pfnUserCallback = debugCallback,
        .pUserData = logger};
    return createInfo;
}

vk::DebugUtilsMessengerEXT createDebugUtilsMessengerEXT(
    vk::Instance instance,
    spdlog::logger* logger)
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
    auto [result, vkDebugMessenger] = instance.createDebugUtilsMessengerEXT(
        getDebugUtilsCreateInfo(createInfo, logger), nullptr);
    EXPENGINE_VK_ASSERT(result, "Debug Utils Messenger could not be created.");

    return vkDebugMessenger;
}

void destroyDebugUtilsMessengerEXT(
    vk::Instance instance,
    vk::DebugUtilsMessengerEXT debugMessenger)
{
    instance.destroyDebugUtilsMessengerEXT(
        debugMessenger, nullptr, VULKAN_HPP_DEFAULT_DISPATCHER);
}

bool hasValidationLayerSupport(const std::vector<const char*> validationLayers)
{
    auto [result, availableLayers] = vk::enumerateInstanceLayerProperties();
    EXPENGINE_VK_ASSERT(result, "Failed to enumerate instance available layers.");

    /* Verify that the layers we want are available */
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
            SPDLOG_INFO("Requested Layer '{}' - Not available", layerName);
            return false;
        }
        else
        {
            SPDLOG_INFO("Requested Layer '{}' - Available", layerName);
        }
    }
    return true;
}

} // namespace vlk
} // namespace experim
