#include "RendererBackendVulkan.hpp"

#include <string>

#include <engine/render/vlk/VlkDebug.hpp>

namespace {
const std::string RENDERER_PALTFORM_NAME = "ExperimEngine_Vulkan_Renderer";
}

namespace expengine {
namespace render {

RendererBackendVulkan::RendererBackendVulkan(
	const vlk::Device& vlkDevice,
	const RenderingContext& mainRenderingContext)
{
	/* ------------------------------------------- */
	/* Setup Renderer capabilities flags           */
	/* ------------------------------------------- */

	/* Setup back-end capabilities flags */
	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = RENDERER_PALTFORM_NAME.c_str();
	/* We can honor the ImDrawCmd::VtxOffset field, allowing for large
	 * meshes. */
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
	/* We can create multi-viewports on the Renderer side */
	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

	/* ------------------------------------------- */
	/* Create device objects                       */
	/* ------------------------------------------- */

	/* Font sampler */
	auto samplerResult = vlkDevice.deviceHandle().createSamplerUnique(
		{ .magFilter = vk::Filter::eLinear,
		  .minFilter = vk::Filter::eLinear,
		  .mipmapMode = vk::SamplerMipmapMode::eLinear,
		  .addressModeU = vk::SamplerAddressMode::eRepeat,
		  .addressModeV = vk::SamplerAddressMode::eRepeat,
		  .addressModeW = vk::SamplerAddressMode::eRepeat,
		  .maxAnisotropy = 1.0f,
		  .minLod = -1000,
		  .maxLod = 1000 });
	EXPENGINE_VK_ASSERT(samplerResult.result,
						"Failed to create font sampler");
	fontSampler_ = std::move(samplerResult.value);

	/* Descriptor set layout */
	vk::DescriptorSetLayoutBinding binding[1]
		= { { .descriptorType = vk::DescriptorType::eCombinedImageSampler,
			  .descriptorCount = 1,
			  .stageFlags = vk::ShaderStageFlagBits::eFragment,
			  .pImmutableSamplers = &fontSampler_.get() } };
	auto descSetLayoutResult
		= vlkDevice.deviceHandle().createDescriptorSetLayoutUnique(
			{ .bindingCount = 1, .pBindings = binding });
	EXPENGINE_VK_ASSERT(descSetLayoutResult.result,
						"Failed to create descriptor set layout");
	descriptorSetLayout_ = std::move(descSetLayoutResult.value);

	/* Descriptor set */
	auto descriptorResult
		= vlkDevice.deviceHandle().allocateDescriptorSets(
			{ .descriptorPool = vlkDevice.descriptorPool(),
			  .descriptorSetCount = 1,
			  .pSetLayouts = &descriptorSetLayout_.get() });
	EXPENGINE_VK_ASSERT(descriptorResult.result,
						"Failed to create descriptor set");
	descriptorSet_ = descriptorResult.value.front();

	/* Pipeline layout */
	vk::PushConstantRange pushConstants[1]
		= { { .stageFlags = vk::ShaderStageFlagBits::eVertex,
			  .offset = 0,
			  .size = sizeof(float) * 4 } };
	auto pipelineLayoutResult
		= vlkDevice.deviceHandle().createPipelineLayoutUnique(
			{ .setLayoutCount = 1,
			  .pSetLayouts = &descriptorSetLayout_.get(),
			  .pushConstantRangeCount = 1,
			  .pPushConstantRanges = pushConstants });
	EXPENGINE_VK_ASSERT(pipelineLayoutResult.result,
						"Failed to create pipeline layout");
	pipelineLayout_ = std::move(pipelineLayoutResult.value);

	/* ------------------------------------------- */
	/* Rendering bindings                          */
	/* ------------------------------------------- */

	/* Bind rendering delegates */
	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		EXPENGINE_ASSERT(
			platform_io.Platform_CreateVkSurface != nullptr,
			"Platform needs to setup the CreateVkSurface handler");
	/* TODO */
	// platform_io.Renderer_CreateWindow = ImGui_ImplVulkan_CreateWindow;
	// platform_io.Renderer_DestroyWindow = ImGui_ImplVulkan_DestroyWindow;
	// platform_io.Renderer_SetWindowSize = ImGui_ImplVulkan_SetWindowSize;
	// platform_io.Renderer_RenderWindow = ImGui_ImplVulkan_RenderWindow;
	// platform_io.Renderer_SwapBuffers = ImGui_ImplVulkan_SwapBuffers;
}

void RendererBackendVulkan::uploadFonts(
	const vlk::Device& vlkDevice, const RenderingContext& renderingContext)
{
}

} // namespace render
} // namespace expengine
