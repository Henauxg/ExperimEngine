#pragma once

#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

class Device;

/* Must be called once, before creating the Vulkan instance. */
void initializeDispatch();

/* Must be called once aftert creating the instance, to get access to all the other
 * function pointers. */
void initializeInstanceDispatch(vk::Instance instance);

/* Optional. Can be called with the chosen device to bypass some intermediate layers
 * and be a bit more efficient. Get device specific function pointers. */
void specializeDeviceDispatch(const Device& device);

} // namespace vlk
} // namespace render
} // namespace expengine