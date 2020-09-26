#pragma once

#include <engine/render/RenderingContext.hpp>
#include <engine/render/Window.hpp>
#include <engine/render/imgui/impl/ImguiContext.hpp>
#include <engine/render/imgui/lib/imgui.h>
#include <engine/render/vlk/VlkDevice.hpp>
#include <engine/render/vlk/resources/VlkTexture.hpp>

namespace expengine {
namespace render {

/* Shared device objects at backend level : */
/* -> Font Texture (Image + ImageView + Memory) */
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
		std::shared_ptr<ImguiContext> context,
		const vlk::Device& vlkDevice,
		std::shared_ptr<RenderingContext> mainRenderingContext);
	~RendererBackendVulkan();

	void uploadFonts(const vlk::Device& vlkDevice);

private:
	/* ImGui */
	const std::shared_ptr<ImguiContext> context_;

	std::unique_ptr<vlk::Texture> fontTexture_;
	vk::UniqueDescriptorSetLayout descriptorSetLayout_;
	/* Not unique since the pool owns it
	 * (VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT not used)  */
	vk::DescriptorSet descriptorSet_;
	vk::UniquePipelineLayout pipelineLayout_;
	vk::UniqueSampler fontSampler_;

	/* Logging */
	std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
