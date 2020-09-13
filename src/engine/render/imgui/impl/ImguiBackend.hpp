#pragma once

#include <memory>

#include <engine/render/imgui/impl/PlatformBackendSDL.hpp>
#include <engine/render/imgui/impl/RendererBackendVulkan.hpp>
#include <engine/render/imgui/lib/imgui.h>

namespace expengine {
namespace render {

/** Custom back-end */
class ImguiBackend {
public:
	ImguiBackend::ImguiBackend(
		const vlk::Device& vlkDevice,
		const RenderingContext& mainRenderingContext,
		std::shared_ptr<Window> window);
	~ImguiBackend();

	/* TODO : Could swap ImGuiContext if multiple contexts are used. */
	inline bool handleEvent(const SDL_Event& event)
	{
		return platformBackend_->handleEvent(event);
	};

private:
	/* ImGui */
	ImGuiContext* ctx_;
	ImFont* fontRegular_;

	/* Platform */
	std::unique_ptr<PlatformBackendSDL> platformBackend_;
	/* Rendering */
	std::unique_ptr<RendererBackendVulkan> renderingBackend_;
};

} // namespace render
} // namespace expengine
