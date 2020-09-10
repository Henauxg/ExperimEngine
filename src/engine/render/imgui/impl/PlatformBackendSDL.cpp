#include "PlatformBackendSDL.hpp"

#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace {
const std::string BACKEND_PALTFORM_NAME = "ExperimEngine_SDL";
}

namespace expengine {
namespace render {

// Helper structure we store in the void* RenderUserData field of each
// ImGuiViewport to easily retrieve our backend data.
struct ImGuiViewportDataSDL2 {
	uint32_t windowId_;
	bool windowOwned_;

	ImGuiViewportDataSDL2()
		: windowId_(0)
		, windowOwned_(false)
	{
	}
	~ImGuiViewportDataSDL2() { }
};

const char* ImGui_ImplSDL2_GetClipboardTextDelegate(void*);
void ImGui_ImplSDL2_SetClipboardTextDelegate(void*, const char* text);

PlatformBackendSDL::PlatformBackendSDL(const Window& window)
	: clipboardTextData_(nullptr)
	, mouseCanUseGlobalState_(true)
{
	/* ------------------------------------------- */
	/* Setup Platform bindings                     */
	/* ------------------------------------------- */

	/* Back-end capabilities flags */
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

	io.BackendPlatformName = BACKEND_PALTFORM_NAME.c_str();

	/* Keyboard mapping. ImGui will use those indices to peek into the
	 * io.KeysDown[] array. */
	io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
	io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
	io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
	io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
	io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
	io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
	io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
	io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
	io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
	io.KeyMap[ImGuiKey_KeyPadEnter] = SDL_SCANCODE_KP_ENTER;
	io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
	io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
	io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
	io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
	io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
	io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

	io.SetClipboardTextFn = ImGui_ImplSDL2_SetClipboardTextDelegate;
	io.GetClipboardTextFn = ImGui_ImplSDL2_GetClipboardTextDelegate;
	io.ClipboardUserData = this;

	/* Load mouse cursors */
	mouseCursors_[ImGuiMouseCursor_Arrow]
		= SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	mouseCursors_[ImGuiMouseCursor_TextInput]
		= SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	mouseCursors_[ImGuiMouseCursor_ResizeAll]
		= SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	mouseCursors_[ImGuiMouseCursor_ResizeNS]
		= SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	mouseCursors_[ImGuiMouseCursor_ResizeEW]
		= SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	mouseCursors_[ImGuiMouseCursor_ResizeNESW]
		= SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	mouseCursors_[ImGuiMouseCursor_ResizeNWSE]
		= SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	mouseCursors_[ImGuiMouseCursor_Hand]
		= SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
	mouseCursors_[ImGuiMouseCursor_NotAllowed]
		= SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);

	/* Check and store if we are on Wayland */
	mouseCanUseGlobalState_
		= strncmp(SDL_GetCurrentVideoDriver(), "wayland", 7) != 0;

	/* ------------------------------------------- */
	/* Setup main viewport/window                  */
	/* ------------------------------------------- */

	/* Mouse update function expect PlatformHandle to be filled for the
	 * main viewport */
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	/* Use an Id instead of the SDL_Window handle to avoid exposing
	 * SDL_Window type as an interface of Window. */
	mainViewport->PlatformHandle
		= ((void*) (uintptr_t) window.getWindowId());
	/* TODO set PlatformHandleRaw ? */

	/* ------------------------------------------- */
	/* Register monitors                           */
	/* ------------------------------------------- */

	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	platform_io.Monitors.resize(0);
	int displayCount = SDL_GetNumVideoDisplays();
	for (int displayIndex = 0; displayIndex < displayCount; displayIndex++)
	{
		/* Warning: the validity of monitor DPI information on Windows
		 * depends on the application DPI awareness settings, which
		 * generally needs to be set in the manifest or at runtime. */
		ImGuiPlatformMonitor monitor;
		SDL_Rect r;
		SDL_GetDisplayBounds(displayIndex, &r);
		monitor.MainPos = monitor.WorkPos
			= ImVec2((float) r.x, (float) r.y);
		monitor.MainSize = monitor.WorkSize
			= ImVec2((float) r.w, (float) r.h);

		SDL_GetDisplayUsableBounds(displayIndex, &r);
		monitor.WorkPos = ImVec2((float) r.x, (float) r.y);
		monitor.WorkSize = ImVec2((float) r.w, (float) r.h);
		float dpi = 0.0f;
		if (!SDL_GetDisplayDPI(displayIndex, &dpi, NULL, NULL))
			monitor.DpiScale = dpi / 96.0f;

		platform_io.Monitors.push_back(monitor);
	}

	/* ------------------------------------------- */
	/* Initialize platform interface               */
	/* ------------------------------------------- */

	if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		&& (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports))
	{
		ImGuiPlatformIO& plt_io = ImGui::GetPlatformIO();
		// TODO Implement platform delegates
		// plt_io.Platform_CreateWindow = ImGui_ImplSDL2_CreateWindow;
		// plt_io.Platform_DestroyWindow = ImGui_ImplSDL2_DestroyWindow;
		// plt_io.Platform_ShowWindow = ImGui_ImplSDL2_ShowWindow;
		// plt_io.Platform_SetWindowPos = ImGui_ImplSDL2_SetWindowPos;
		// plt_io.Platform_GetWindowPos = ImGui_ImplSDL2_GetWindowPos;
		// plt_io.Platform_SetWindowSize = ImGui_ImplSDL2_SetWindowSize;
		// plt_io.Platform_GetWindowSize = ImGui_ImplSDL2_GetWindowSize;
		// plt_io.Platform_SetWindowFocus = ImGui_ImplSDL2_SetWindowFocus;
		// plt_io.Platform_GetWindowFocus = ImGui_ImplSDL2_GetWindowFocus;
		// plt_io.Platform_GetWindowMinimized
		//	= ImGui_ImplSDL2_GetWindowMinimized;
		// plt_io.Platform_SetWindowTitle = ImGui_ImplSDL2_SetWindowTitle;
		// plt_io.Platform_RenderWindow = ImGui_ImplSDL2_RenderWindow;
		// plt_io.Platform_SwapBuffers = ImGui_ImplSDL2_SwapBuffers;
		// plt_io.Platform_SetWindowAlpha = ImGui_ImplSDL2_SetWindowAlpha;
		// plt_io.Platform_CreateVkSurface =
		// ImGui_ImplSDL2_CreateVkSurface;

		/* SDL2 by default doesn't pass mouse clicks to the application
		 * when the click focused a window. This is getting in the way of
		 * our interactions and we disable that behavior. */
		SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

		/* Register main window handle (which is owned by the main
		 * application, not by imgui) Mostly for simplicity and
		 * consistency, so that backend code (e.g. mouse handling etc.) can
		 * use same logic for main and secondary viewports. */
		ImGuiViewportDataSDL2* data = IM_NEW(ImGuiViewportDataSDL2)();
		data->windowId_ = window.getWindowId();
		data->windowOwned_ = false;
		mainViewport->PlatformUserData = data;
	}
}

void PlatformBackendSDL::eraseClipboardData()
{
	if (clipboardTextData_)
		SDL_free(clipboardTextData_);
}

const char* PlatformBackendSDL::getClipboardData()
{
	clipboardTextData_ = SDL_GetClipboardText();
	return clipboardTextData_;
}

const char* ImGui_ImplSDL2_GetClipboardTextDelegate(void* userData)
{
	PlatformBackendSDL* backend
		= static_cast<PlatformBackendSDL*>(userData);
	backend->eraseClipboardData();
	return backend->getClipboardData();
}

void ImGui_ImplSDL2_SetClipboardTextDelegate(void* userData,
											 const char* text)
{
	SDL_SetClipboardText(text);
}

} // namespace render
} // namespace expengine