#pragma once

#include <SDL2\SDL_events.h>

#include <engine/render/Window.hpp>
#include <engine/render/imgui/lib/imgui.h>

struct SDL_Cursor;

namespace expengine {
namespace render {

/** Custom back-end based on imgui_impl_sdl */
class PlatformBackendSDL {
public:
	PlatformBackendSDL::PlatformBackendSDL(std::shared_ptr<Window> window);

	void eraseClipboardData();
	const char* getClipboardData();
	bool handleEvent(const SDL_Event& event);

private:
	/* Platform */
	char* clipboardTextData_;
	bool mouseCanUseGlobalState_;
	SDL_Cursor* mouseCursors_[ImGuiMouseCursor_COUNT];
};

} // namespace render
} // namespace expengine
