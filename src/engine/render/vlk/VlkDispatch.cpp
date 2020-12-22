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

vk::DispatchLoaderDynamic& initializeDispatch()
{
    EXPENGINE_ASSERT(!g_dispatchInitialized, "Dispatch already initialized");

    g_dispatchInitialized = true;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
        = g_dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    return VULKAN_HPP_DEFAULT_DISPATCHER;
}

void initializeInstanceDispatch(
    vk::Instance instance,
    vk::DispatchLoaderDynamic& dispatchloader)
{
    dispatchloader.init(instance);
}

void specializeDeviceDispatch(
    const Device& device,
    vk::DispatchLoaderDynamic& dispatchloader)
{
    dispatchloader.init(device.deviceHandle());
}

} // namespace vlk
} // namespace render
} // namespace expengine