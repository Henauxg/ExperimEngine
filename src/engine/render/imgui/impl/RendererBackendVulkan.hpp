#pragma once

#include <engine/render/RenderingContext.hpp>
#include <engine/render/Window.hpp>
#include <engine/render/imgui/lib/imgui.h>
#include <engine/render/vlk/VlkDevice.hpp>

namespace expengine {
namespace render {

/* Shared device objects at backend level : */
/* -> Sampler (font) */
/* -> Descriptor set layout */
/* -> Descriptor set */
/* -> Pipeline layout */

/* Per RenderingContext :*/
/* -> SwapChain */
/* -> Render pass */
/* -> Graphics pipeline */
/* -> Per Image (image count Backbuffers) */
/* --> Command pool */
/* --> Command buffer */
/* --> Fence */
/* --> Semaphores (x2) */
/* --> Image view  (BackbufferView)*/
/* --> Framebuffer */

/** Custom back-end inspired by imgui_impl_vulkan. */
class RendererBackendVulkan {
public:
	RendererBackendVulkan::RendererBackendVulkan(
		const vlk::Device& vlkDevice,
		const RenderingContext& mainRenderingContext);

	void uploadFonts(const vlk::Device& vlkDevice,
					 const RenderingContext& renderingContext);

private:
	vk::UniqueSampler fontSampler_;
	vk::UniqueDescriptorSetLayout descriptorSetLayout_;
	vk::UniquePipelineLayout pipelineLayout_;
	/* Not unique since the pool owns it
	 * (VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT not used)  */
	vk::DescriptorSet descriptorSet_;
};

} // namespace render
} // namespace expengine
