#include "ImGuiBackend.hpp"

#include <engine/log/ExpengineLog.hpp>

namespace {

// TODO Cross-platform
const std::string OPEN_SANS_FONT
	= "./data/fonts/OpenSans/OpenSans-Regular.ttf";

} // namespace

namespace expengine {
namespace render {

ImguiBackend::ImguiBackend(const vlk::Device& vlkDevice,
						   std::shared_ptr<Window> window)
	: logger_(spdlog::get(LOGGER_NAME))
{
	/* ------------------------------------------- */
	/* Setup Dear ImGui context                    */
	/* ------------------------------------------- */

	IMGUI_CHECKVERSION();
	/* Shared ownership with platform and rendering bakcends */
	context_ = std::make_shared<ImGuiContextWrapper>();
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

	platformBackend_
		= std::make_unique<UIPlatformBackendSDL>(context_, window);

	/* ------------------------------------------- */
	/* Setup Renderer bindings                     */
	/* ------------------------------------------- */

	renderingBackend_
		= std::make_unique<UIRendererBackendVulkan>(context_, vlkDevice);

	/* ------------------------------------------- */
	/* Fonts loading & Uploading                   */
	/* ------------------------------------------- */

	/* Load */
	fontRegular_
		= io.Fonts->AddFontFromFileTTF(OPEN_SANS_FONT.c_str(), 17.0f);
	EXPENGINE_ASSERT(fontRegular_ != nullptr, "Failed to load font : {}",
					 OPEN_SANS_FONT);

	/* Upload to GPU */
	renderingBackend_->uploadFonts(vlkDevice);
}

ImguiBackend::~ImguiBackend()
{
	SPDLOG_LOGGER_DEBUG(logger_, "ImguiBackend destruction");
}

} // namespace render
} // namespace expengine
