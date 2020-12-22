#pragma once

#include <vma/vk_mem_alloc.h>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

class MemoryAllocator {
public:
    /**
     * @brief Create the VMA allocator
     *
     * @param vulkanApiVersion It must be a value in the format as created by macro
     * VK_MAKE_VERSION or a constant like: VK_API_VERSION_1_1, VK_API_VERSION_1_0.
     * The patch version number specified is ignored. Only the major and minor
     * versions are considered. It must be less or equal (preferably equal) to value
     * as passed to vkCreateInstance as VkApplicationInfo::apiVersion. Only
     * versions 1.0, 1.1, 1.2 are supported by the current implementation.
     */
    MemoryAllocator(
        vk::Instance instance,
        const Device& device,
        const vk::DispatchLoaderDynamic& dispatchLoader,
        uint32_t vulkanApiVersion);
    ~MemoryAllocator();

private:
    VmaAllocator allocator_;

    /* Logging */
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace vlk
} // namespace render
} // namespace expengine
