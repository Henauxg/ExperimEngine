#pragma once

#include <engine/render/vlk/VlkInclude.hpp>

namespace expengine {
namespace render {
namespace vlk {

/**
 * @brief Put an image memory barrier for setting an image layout on the
 * sub resource into the given command buffer
 */
void transitionImageLayout(vk::CommandBuffer commandBuffer,
						   vk::Image image, vk::ImageLayout oldLayout,
						   vk::ImageLayout newLayout,
						   vk::ImageSubresourceRange subresourceRange,
						   vk::PipelineStageFlags srcMask
						   = vk::PipelineStageFlagBits::eAllCommands,
						   vk::PipelineStageFlags dstMask
						   = vk::PipelineStageFlagBits::eAllCommands);

/**
 * @brief Put an image memory barrier for setting an image layout into the
 * given command buffer. Fixed ImageSubresourceRange on first mip level and
 * layer
 */
void transitionImageLayout(vk::CommandBuffer commandBuffer,
						   vk::Image image, vk::ImageLayout oldLayout,
						   vk::ImageLayout newLayout,
						   vk::ImageAspectFlags aspectMask,
						   vk::PipelineStageFlags srcMask
						   = vk::PipelineStageFlagBits::eAllCommands,
						   vk::PipelineStageFlags dstMask
						   = vk::PipelineStageFlagBits::eAllCommands);

} // namespace vlk
} // namespace render
} // namespace expengine
