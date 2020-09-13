#include "ImguiBackend.hpp"

#include <engine/log/ExpengineLog.hpp>

namespace {

// TODO Cross-platform
const std::string OPEN_SANS_FONT
	= "./data/fonts/OpenSans/OpenSans-Regular.ttf";

} // namespace

namespace expengine {
namespace render {

ImguiBackend::ImguiBackend(const vlk::Device& vlkDevice,
						   const RenderingContext& mainRenderingContext,
						   std::shared_ptr<Window> window)
{
	/* ------------------------------------------- */
	/* Setup Dear ImGui context                    */
	/* ------------------------------------------- */

	IMGUI_CHECKVERSION();
	ctx_ = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	/* TODO : May look into g.IO.ConfigViewportsNoTaskBarIcon */
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

	/* ------------------------------------------- */
	/* Setup Dear ImGui style                      */
	/* ------------------------------------------- */

	ImGui::StyleColorsDark();

	/* When viewports are enabled WindowRounding/WindowBg are tweaked so
	 * platform windows can look identical to regular ones. */
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	/* ------------------------------------------- */
	/* Setup Platform bindings                     */
	/* ------------------------------------------- */

	platformBackend_ = std::make_unique<PlatformBackendSDL>(window);

	/* ------------------------------------------- */
	/* Setup Renderer bindings                     */
	/* ------------------------------------------- */

	renderingBackend_ = std::make_unique<RendererBackendVulkan>(
		vlkDevice, mainRenderingContext);

	/* ------------------------------------------- */
	/* Fonts loading & Uploading                   */
	/* ------------------------------------------- */

	/* Load */
	fontRegular_
		= io.Fonts->AddFontFromFileTTF(OPEN_SANS_FONT.c_str(), 17.0f);
	EXPENGINE_ASSERT(fontRegular_ != nullptr, "Failed to load font : {}",
					 OPEN_SANS_FONT);

	/* Upload using the main RenderingContext resources */
	renderingBackend_->uploadFonts(vlkDevice, mainRenderingContext);
}

ImguiBackend::~ImguiBackend() { ImGui::DestroyContext(ctx_); }

} // namespace render
} // namespace expengine
