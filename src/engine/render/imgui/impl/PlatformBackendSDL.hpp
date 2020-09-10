#pragma once

#include <engine/render/Window.hpp>
#include <engine/render/imgui/lib/imgui.h>

namespace expengine {
namespace render {

/** Custom back-end based on imgui_impl_sdl */
class PlatformBackendSDL {
public:
	PlatformBackendSDL::PlatformBackendSDL(const Window& window);

	void eraseClipboardData();
	const char* getClipboardData();

private:
	/* Platform */
	char* clipboardTextData_;
	bool mouseCanUseGlobalState_;
	SDL_Cursor* mouseCursors_[ImGuiMouseCursor_COUNT];
};

} // namespace render
} // namespace expengine