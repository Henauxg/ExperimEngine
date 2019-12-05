#pragma once 

#include <vector>

namespace expengine {
namespace render {
namespace vlk {

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

bool hasValidationLayerSupport(const std::vector<const char*> validationLayers);

} // namespace vlk
} // namespace render
} // namespace expengine
