#include "VlkDispatch.hpp"

#include <engine/render/vlk/VlkDevice.hpp>

/* See
 * https://github.com/KhronosGroup/Vulkan-Hpp/blob/master/README.md#extensions--per-device-function-pointers
 * Provide storage here for the default dispatcher */
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace {
vk::DynamicLoader g_dl;
bool g_dispatchInitialized = false;
} // namespace

namespace expengine {
namespace render {
namespace vlk {

void initializeDispatch()
{
    EXPENGINE_ASSERT(!g_dispatchInitialized, "Dispacth already initialized");

    g_dispatchInitialized = true;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
        = g_dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
}

void initializeInstanceDispatch(vk::Instance instance)
{
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
}

void specializeDeviceDispatch(const Device& device)
{
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device.deviceHandle());
}

} // namespace vlk
} // namespace render
} // namespace expengine