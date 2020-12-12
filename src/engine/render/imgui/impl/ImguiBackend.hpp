#pragma once

#include <memory>

#include <engine/render/imgui/impl/ImGuiContextWrapper.hpp>
#include <engine/render/imgui/impl/UIPlatformBackendSDL.hpp>
#include <engine/render/imgui/impl/UIRendererBackendVulkan.hpp>
#include <engine/render/imgui/lib/imgui.h>

namespace expengine {
namespace render {

class RenderingContext;

/** Custom back-end */
class ImguiBackend {
public:
	ImguiBackend(const vlk::Device& vlkDevice,
				 std::shared_ptr<Window> window);
	~ImguiBackend();

	/* TODO : Could swap ImGuiContext if multiple contexts are used. */
	inline bool handleEvent(const SDL_Event& event)
	{
		return platformBackend_->handleEvent(event);
	};
	/* TODO Multi-Rendering-Backends : May need to make this an interface
	 */
	inline const UIRendererBackendVulkan& getRenderingBackend() const
	{
		return *renderingBackend_;
	};

	inline void bindMainRenderingContext(
		std::shared_ptr<RenderingContext> mainRenderingContext)
	{
		renderingBackend_->bindMainRenderingContext(mainRenderingContext);
	}

private:
	/* ImGui */
	std::shared_ptr<ImGuiContextWrapper> context_;
	ImFont* fontRegular_;

	/* Platform */
	std::unique_ptr<UIPlatformBackendSDL> platformBackend_;
	/* Rendering */
	std::unique_ptr<UIRendererBackendVulkan> renderingBackend_;

	/* Logging */
	std::shared_ptr<spdlog::logger> logger_;
};

} // namespace render
} // namespace expengine
