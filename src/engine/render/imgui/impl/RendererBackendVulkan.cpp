#include "RendererBackendVulkan.hpp"

#include <string>

#include <engine/render/imgui/impl/PlatformBackendData.hpp>
#include <engine/render/vlk/VlkDebug.hpp>

namespace {
const std::string RENDERER_BACKEND_NAME = "ExperimEngine_Vulkan_Renderer";
}

namespace expengine {
namespace render {

/* Static used to give acces to vulkan device/instance to imgui static
 * callbacks. Device is destroyed after imgui backend and thus should not
 * be able to be null.
 */
static const vlk::Device* gVlkDevice = nullptr;

/* Helper structure stored in the void* RenderUserData field of each
 * ImGuiViewport to easily retrieve rendering backend data. */
struct ImGuiViewportRendererData {
	std::shared_ptr<RenderingContext> renderingContext_;

	ImGuiViewportRendererData(
		std::shared_ptr<RenderingContext> renderingContext)
		: renderingContext_(renderingContext)
	{
	}
	~ImGuiViewportRendererData() { }
};

/* Delegates */
static void ImGui_ImplExpengine_CreateWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport);

RendererBackendVulkan::RendererBackendVulkan(
	std::shared_ptr<ImguiContext> context, const vlk::Device& vlkDevice,
	std::shared_ptr<RenderingContext> mainRenderingContext)
	: context_(context)
	, logger_(spdlog::get(LOGGER_NAME))
{
	/* ------------------------------------------- */
	/* Setup Renderer capabilities flags           */
	/* ------------------------------------------- */

	/* Setup back-end capabilities flags */
	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = RENDERER_BACKEND_NAME.c_str();
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

	/* TODO once in RendererBackendVulkan
	 * Create ImGui shaders modules
	 */

	/* ------------------------------------------- */
	/* Rendering bindings                          */
	/* ------------------------------------------- */

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		/* Give delegates access to vulkan device */
		gVlkDevice = &vlkDevice;

		/* Bind rendering delegates */
		ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		/* No check for Platform_CreateVkSurface since handled by RC */
		platform_io.Renderer_CreateWindow
			= ImGui_ImplExpengine_CreateWindow;
		/* Note : also called on main viewport */
		platform_io.Renderer_DestroyWindow
			= ImGui_ImplExpengine_DestroyWindow;
		/* TODO */
		// platform_io.Renderer_SetWindowSize =
		// ImGui_ImplVulkan_SetWindowSize;
		// platform_io.Renderer_RenderWindow =
		// ImGui_ImplVulkan_RenderWindow; platform_io.Renderer_SwapBuffers
		// = ImGui_ImplVulkan_SwapBuffers;
	}

	/* Setup main viewport RendererUserData */
	/* Note : cleaned by ImGui_ImplExpengine_DestroyWindow if viewport
	 * enabled. Else cleaned by RendererBackend */
	ImGuiViewportRendererData* data
		= new ImGuiViewportRendererData(mainRenderingContext);
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	mainViewport->RendererUserData = data;
}

RendererBackendVulkan::~RendererBackendVulkan()
{
	SPDLOG_LOGGER_DEBUG(logger_, "RendererBackendVulkan destruction");
	/* Clean main viewport render data if viewport is not enabled */
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	if (ImGuiViewportRendererData* data
		= (ImGuiViewportRendererData*) mainViewport->RendererUserData)
		delete data;
	mainViewport->RendererUserData = nullptr;
}

void RendererBackendVulkan::uploadFonts(const vlk::Device& vlkDevice)
{
	/* Get texture data from ImGui */
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixelsBuffer;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixelsBuffer, &width, &height);
	size_t bufferSize
		= (size_t) width * (size_t) height * 4 * sizeof(char);

	/* Create GPU texture */
	fontTexture_ = std::make_unique<vlk::Texture>(
		vlkDevice, pixelsBuffer, bufferSize, vk::Format::eR8G8B8A8Unorm,
		width, height, fontSampler_.get(),
		vk::ImageUsageFlagBits::eSampled
			| vk::ImageUsageFlagBits::eTransferDst,
		vk::ImageLayout::eShaderReadOnlyOptimal);

	/* Store font texture identifier */
	io.Fonts->TexID
		= (ImTextureID)(intptr_t)(VkImage) fontTexture_->imageHandle();
}

static void ImGui_ImplExpengine_CreateWindow(ImGuiViewport* viewport)
{
	/* Get window from platform data */
	auto platformData
		= (ImGuiViewportPlatformData*) viewport->PlatformUserData;
	EXPENGINE_ASSERT(platformData != nullptr,
					 "Error, null PlatformUserData");

	/* Get instance and device from module RendererBackendVulkan global */
	EXPENGINE_ASSERT(gVlkDevice != nullptr, "Error, null gVlkDevice");

	/* Create a RenderingContext. Surface creation is handled by the RC. */
	/* TODO Create Render pass attachments here and send to the RC */
	auto renderingContext = std::make_shared<RenderingContext>(
		gVlkDevice->instanceHandle(), *gVlkDevice, platformData->window_);

	/* Allocate RendererUserData */
	auto data = new ImGuiViewportRendererData(renderingContext);
	viewport->RendererUserData = data;
}

static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport)
{
	auto data = (ImGuiViewportRendererData*) viewport->RendererUserData;
	if (data)
	{
		delete data;
	}
	viewport->RendererUserData = nullptr;
}

} // namespace render
} // namespace expengine
