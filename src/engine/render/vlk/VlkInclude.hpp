#pragma once

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL 1
#define VULKAN_HPP_NO_EXCEPTIONS
/* https://github.com/KhronosGroup/Vulkan-Hpp#designated-initializers */
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

const static uint32_t ENGINE_VULKAN_API_VERSION = VK_API_VERSION_1_0;
