#include "PlatformBackendSDL.hpp"

#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

namespace {
const std::string BACKEND_PALTFORM_NAME = "ExperimEngine_SDL";
}

namespace expengine {
namespace render {

/* Helper structure we store in the void* RenderUserData field of each
 * ImGuiViewport to easily retrieve our backend data. */
struct ImGuiViewportData {
	std::shared_ptr<Window> window_;

	ImGuiViewportData(std::shared_ptr<Window> window)
		: window_(window)
	{
	}
	~ImGuiViewportData() { }
};

/* Delegates */
static const char* ImGui_ImplExpengine_GetClipboardText(void*);
static void ImGui_ImplSDL2_SetClipboardText(void*, const char* text);
static void ImGui_ImplSDL2_CreateWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_ShowWindow(ImGuiViewport* viewport);
static ImVec2 ImGui_ImplExpengine_GetWindowPos(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_SetWindowPos(ImGuiViewport* viewport,
											 ImVec2 pos);
static ImVec2 ImGui_ImplExpengine_GetWindowSize(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_SetWindowSize(ImGuiViewport* viewport,
											  ImVec2 size);
static void ImGui_ImplExpengine_SetWindowTitle(ImGuiViewport* viewport,
											   const char* title);
static void ImGui_ImplExpengine_SetWindowAlpha(ImGuiViewport* viewport,
											   float alpha);
static void ImGui_ImplExpengine_SetWindowFocus(ImGuiViewport* viewport);
static bool ImGui_ImplExpengine_GetWindowFocus(ImGuiViewport* viewport);
static bool
ImGui_ImplExpengine_GetWindowMinimized(ImGuiViewport* viewport);
static int ImGui_ImplExpengine_CreateVkSurface(ImGuiViewport* viewport,
											   ImU64 vkInstance,
											   const void* vkAllocator,
											   ImU64* outSurface);

PlatformBackendSDL::PlatformBackendSDL(std::shared_ptr<Window> window)
	: clipboardTextData_(nullptr)
	, mouseCanUseGlobalState_(true)
	, mousePressed_({ false, false, false })
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

	io.SetClipboardTextFn = ImGui_ImplSDL2_SetClipboardText;
	io.GetClipboardTextFn = ImGui_ImplExpengine_GetClipboardText;
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
	mainViewport->PlatformHandle = window->getPlatformHandle();
	mainViewport->PlatformHandleRaw = window->getPlatformHandleRaw();

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
	/* TODO : It seems that SDL does not send events for display/monitor
	 * updates ?
	 */

	/* ------------------------------------------- */
	/* Initialize platform interface               */
	/* ------------------------------------------- */

	if ((io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		&& (io.BackendFlags & ImGuiBackendFlags_PlatformHasViewports))
	{
		ImGuiPlatformIO& plt_io = ImGui::GetPlatformIO();

		plt_io.Platform_CreateWindow = ImGui_ImplSDL2_CreateWindow;
		plt_io.Platform_DestroyWindow = ImGui_ImplExpengine_DestroyWindow;
		plt_io.Platform_ShowWindow = ImGui_ImplExpengine_ShowWindow;
		plt_io.Platform_SetWindowPos = ImGui_ImplExpengine_SetWindowPos;
		plt_io.Platform_GetWindowPos = ImGui_ImplExpengine_GetWindowPos;
		plt_io.Platform_SetWindowSize = ImGui_ImplExpengine_SetWindowSize;
		plt_io.Platform_GetWindowSize = ImGui_ImplExpengine_GetWindowSize;
		plt_io.Platform_SetWindowFocus
			= ImGui_ImplExpengine_SetWindowFocus;
		plt_io.Platform_GetWindowFocus
			= ImGui_ImplExpengine_GetWindowFocus;
		plt_io.Platform_GetWindowMinimized
			= ImGui_ImplExpengine_GetWindowMinimized;
		plt_io.Platform_SetWindowTitle
			= ImGui_ImplExpengine_SetWindowTitle;
		plt_io.Platform_SetWindowAlpha
			= ImGui_ImplExpengine_SetWindowAlpha;
		plt_io.Platform_CreateVkSurface
			= ImGui_ImplExpengine_CreateVkSurface;
		/* TODO : Unused with Vulkan. No OpenGl backend planned. */
		/* plt_io.Platform_RenderWindow = */
		/* plt_io.Platform_SwapBuffers = */

		/* SDL2 by default doesn't pass mouse clicks to the application
		 * when the click focused a window. This is getting in the way of
		 * our interactions and we disable that behavior. */
		SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

		ImGuiViewportData* data = new ImGuiViewportData(window);
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

bool PlatformBackendSDL::handleEvent(const SDL_Event& event)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (event.type)
	{
	case SDL_MOUSEWHEEL: {
		if (event.wheel.x > 0)
			io.MouseWheelH += 1;
		if (event.wheel.x < 0)
			io.MouseWheelH -= 1;
		if (event.wheel.y > 0)
			io.MouseWheel += 1;
		if (event.wheel.y < 0)
			io.MouseWheel -= 1;
		return true;
	}
	case SDL_MOUSEBUTTONDOWN: {
		if (event.button.button == SDL_BUTTON_LEFT)
			mousePressed_[0] = true;
		if (event.button.button == SDL_BUTTON_RIGHT)
			mousePressed_[1] = true;
		if (event.button.button == SDL_BUTTON_MIDDLE)
			mousePressed_[2] = true;
		return true;
	}
	case SDL_TEXTINPUT: {
		io.AddInputCharactersUTF8(event.text.text);
		return true;
	}
	case SDL_KEYDOWN:
	case SDL_KEYUP: {
		int key = event.key.keysym.scancode;
		IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
		io.KeysDown[key] = (event.type == SDL_KEYDOWN);
		io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
		io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
		io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
#ifdef _WIN32
		io.KeySuper = false;
#else
		io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
#endif
		return true;
	}
	/* Multi-viewport support */
	case SDL_WINDOWEVENT:
		Uint8 windowEvent = event.window.event;
		if (windowEvent == SDL_WINDOWEVENT_CLOSE
			|| windowEvent == SDL_WINDOWEVENT_MOVED
			|| windowEvent == SDL_WINDOWEVENT_RESIZED)
			if (ImGuiViewport* viewport
				= ImGui::FindViewportByPlatformHandle(
					(void*) SDL_GetWindowFromID(event.window.windowID)))
			{
				if (windowEvent == SDL_WINDOWEVENT_CLOSE)
					viewport->PlatformRequestClose = true;
				if (windowEvent == SDL_WINDOWEVENT_MOVED)
					viewport->PlatformRequestMove = true;
				if (windowEvent == SDL_WINDOWEVENT_RESIZED)
					viewport->PlatformRequestResize = true;
				return true;
			}
		break;
	}
	return false;
}

static const char* ImGui_ImplExpengine_GetClipboardText(void* userData)
{
	auto backend = static_cast<PlatformBackendSDL*>(userData);
	backend->eraseClipboardData();
	return backend->getClipboardData();
}

static void ImGui_ImplSDL2_SetClipboardText(void* userData,
											const char* text)
{
	SDL_SetClipboardText(text);
}

static void ImGui_ImplSDL2_CreateWindow(ImGuiViewport* viewport)
{
	Uint32 sdl_flags = 0;
	sdl_flags |= SDL_WINDOW_VULKAN;
	sdl_flags |= SDL_WINDOW_ALLOW_HIGHDPI;
	sdl_flags |= SDL_WINDOW_HIDDEN;
	sdl_flags |= (viewport->Flags & ImGuiViewportFlags_NoDecoration)
		? SDL_WINDOW_BORDERLESS
		: 0;
	sdl_flags |= (viewport->Flags & ImGuiViewportFlags_NoDecoration)
		? 0
		: SDL_WINDOW_RESIZABLE;
#if !defined(_WIN32)
	/* See SDL hack in ImGui_ImplExpengine_ShowWindow() */
	sdl_flags |= (viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
		? SDL_WINDOW_SKIP_TASKBAR
		: 0;
#endif
	sdl_flags |= (viewport->Flags & ImGuiViewportFlags_TopMost)
		? SDL_WINDOW_ALWAYS_ON_TOP
		: 0;

	auto window = std::make_shared<render::Window>(
		(int) viewport->Size.x, (int) viewport->Size.y, "No Title Yet",
		sdl_flags);
	window->setPosition((int) viewport->Pos.x, (int) viewport->Pos.y);

	auto data = new ImGuiViewportData(window);
	viewport->PlatformUserData = data;
	viewport->PlatformHandle = data->window_->getPlatformHandle();
	viewport->PlatformHandleRaw = data->window_->getPlatformHandleRaw();
}

static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
	delete data;
	viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void ImGui_ImplExpengine_ShowWindow(ImGuiViewport* viewport)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
#if defined(_WIN32)
	HWND hwnd = (HWND) viewport->PlatformHandleRaw;

	/* SDL hack: Hide icon from task bar. Note: SDL 2.0.6+ has a
	 * SDL_WINDOW_SKIP_TASKBAR flag which is supported under Windows but
	 * the way it create the window breaks the seamless transition. */
	if (viewport->Flags & ImGuiViewportFlags_NoTaskBarIcon)
	{
		LONG ex_style = ::GetWindowLong(hwnd, GWL_EXSTYLE);
		ex_style &= ~WS_EX_APPWINDOW;
		ex_style |= WS_EX_TOOLWINDOW;
		::SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
	}

	/* SDL hack: SDL always activate/focus windows :/ */
	if (viewport->Flags & ImGuiViewportFlags_NoFocusOnAppearing)
	{
		::ShowWindow(hwnd, SW_SHOWNA);
		return;
	}
#endif
	data->window_->show();
}

static ImVec2 ImGui_ImplExpengine_GetWindowPos(ImGuiViewport* viewport)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
	auto [x, y] = data->window_->getPosition();
	return ImVec2((float) x, (float) y);
}

static void ImGui_ImplExpengine_SetWindowPos(ImGuiViewport* viewport,
											 ImVec2 pos)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
	data->window_->setPosition((int) pos.x, (int) pos.y);
}

static ImVec2 ImGui_ImplExpengine_GetWindowSize(ImGuiViewport* viewport)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
	auto [w, h] = data->window_->getPosition();
	return ImVec2((float) w, (float) h);
}

static void ImGui_ImplExpengine_SetWindowSize(ImGuiViewport* viewport,
											  ImVec2 size)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
	data->window_->setSize((int) size.x, (int) size.y);
}

static void ImGui_ImplExpengine_SetWindowTitle(ImGuiViewport* viewport,
											   const char* title)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
	data->window_->setTitle(title);
}

static void ImGui_ImplExpengine_SetWindowAlpha(ImGuiViewport* viewport,
											   float alpha)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
	data->window_->setOpacity(alpha);
}

static void ImGui_ImplExpengine_SetWindowFocus(ImGuiViewport* viewport)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
	data->window_->setFocus();
}

static bool ImGui_ImplExpengine_GetWindowFocus(ImGuiViewport* viewport)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
	return data->window_->isFocused();
}

static bool ImGui_ImplExpengine_GetWindowMinimized(ImGuiViewport* viewport)
{
	auto data = (ImGuiViewportData*) viewport->PlatformUserData;
	return data->window_->isMinimized();
}

static int ImGui_ImplExpengine_CreateVkSurface(ImGuiViewport* viewport,
											   ImU64 vkInstance,
											   const void* vkAllocator,
											   ImU64* outSurface)
{
	auto* data = (ImGuiViewportData*) viewport->PlatformUserData;
	(void) vkAllocator;
	bool createStatus = data->window_->createVkSurface(
		(VkInstance) vkInstance, *((vk::SurfaceKHR*) outSurface));
	return createStatus;
}

} // namespace render
} // namespace expengine
