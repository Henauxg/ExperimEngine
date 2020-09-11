#pragma once

#include <memory>

#include <engine/render/imgui/impl/PlatformBackendSDL.hpp>
#include <engine/render/imgui/lib/imgui.h>

namespace expengine {
namespace render {

/** Custom back-end */
class ImguiBackend {
public:
	ImguiBackend::ImguiBackend(std::shared_ptr<Window> window);

private:
	/* ImGui */
	ImGuiContext* ctx_;
	ImFont* fontRegular_;

	/* Platform */
	std::unique_ptr<PlatformBackendSDL> platform_;
};

} // namespace render
} // namespace expengine
