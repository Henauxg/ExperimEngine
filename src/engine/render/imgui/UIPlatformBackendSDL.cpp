#include "UIPlatformBackendSDL.hpp"

#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_syswm.h>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/Window.hpp>
#include <engine/render/imgui/ImGuiContextWrapper.hpp>
#include <engine/render/imgui/ImGuiViewportPlatformData.hpp>

#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE SDL_VERSION_ATLEAST(2, 0, 4)

namespace {

const std::string PLATFORM_BACKEND_NAME = "ExperimEngine_SDL_Platform";

} // namespace

namespace expengine {
namespace render {

/* Delegates */
static const char* ImGui_ImplExpengine_GetClipboardText(void*);
static void ImGui_ImplSDL2_SetClipboardText(void*, const char* text);
static void ImGui_ImplSDL2_CreateWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_ShowWindow(ImGuiViewport* viewport);
static ImVec2 ImGui_ImplExpengine_GetWindowPos(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos);
static ImVec2 ImGui_ImplExpengine_GetWindowSize(ImGuiViewport* viewport);
static void ImGui_ImplExpengine_SetWindowSize(ImGuiViewport* viewport, ImVec2 size);
static void ImGui_ImplExpengine_SetWindowTitle(
    ImGuiViewport* viewport,
    const char* title);
static void ImGui_ImplExpengine_SetWindowAlpha(ImGuiViewport* viewport, float alpha);
static void ImGui_ImplExpengine_SetWindowFocus(ImGuiViewport* viewport);
static bool ImGui_ImplExpengine_GetWindowFocus(ImGuiViewport* viewport);
static bool ImGui_ImplExpengine_GetWindowMinimized(ImGuiViewport* viewport);

UIPlatformBackendSDL::UIPlatformBackendSDL(
    std::shared_ptr<ImGuiContextWrapper> imguiContext,
    std::shared_ptr<Window> mainWindow)
    : imguiContext_(imguiContext)
    , mainWindow_(mainWindow)
    , performanceCounterFrequency_(0)
    , previousTime_(0)
    , clipboardTextData_(nullptr)
    , mouseCanUseGlobalState_(true)
    , mousePressed_({false, false, false})
    , logger_(spdlog::get(LOGGER_NAME))
{
    /* ------------------------------------------- */
    /* Setup Platform bindings                     */
    /* ------------------------------------------- */

    /* Back-end capabilities flags */
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;

    io.BackendPlatformName = PLATFORM_BACKEND_NAME.c_str();

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
    mainViewport->PlatformHandle = mainWindow->getPlatformHandle();
    mainViewport->PlatformHandleRaw = mainWindow->getPlatformHandleRaw();

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
        monitor.MainPos = monitor.WorkPos = ImVec2((float) r.x, (float) r.y);
        monitor.MainSize = monitor.WorkSize = ImVec2((float) r.w, (float) r.h);

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
        /* Note : also called on main viewport */
        plt_io.Platform_DestroyWindow = ImGui_ImplExpengine_DestroyWindow;
        plt_io.Platform_ShowWindow = ImGui_ImplExpengine_ShowWindow;
        plt_io.Platform_SetWindowPos = ImGui_ImplExpengine_SetWindowPos;
        plt_io.Platform_GetWindowPos = ImGui_ImplExpengine_GetWindowPos;
        plt_io.Platform_SetWindowSize = ImGui_ImplExpengine_SetWindowSize;
        plt_io.Platform_GetWindowSize = ImGui_ImplExpengine_GetWindowSize;
        plt_io.Platform_SetWindowFocus = ImGui_ImplExpengine_SetWindowFocus;
        plt_io.Platform_GetWindowFocus = ImGui_ImplExpengine_GetWindowFocus;
        plt_io.Platform_GetWindowMinimized = ImGui_ImplExpengine_GetWindowMinimized;
        plt_io.Platform_SetWindowTitle = ImGui_ImplExpengine_SetWindowTitle;
        plt_io.Platform_SetWindowAlpha = ImGui_ImplExpengine_SetWindowAlpha;
        /* Surface creation handled by the RenderingContext */
        /* TODO : Unused with Vulkan. No OpenGl backend planned. */
        /* plt_io.Platform_RenderWindow = */
        /* plt_io.Platform_SwapBuffers = */

        /* SDL2 by default doesn't pass mouse clicks to the application
         * when the click focused a window. This is getting in the way of
         * our interactions and we disable that behavior. */
        SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

        /* Note : cleared by ImGui_ImplExpengine_DestroyWindow */
        ImGuiViewportPlatformData* data = new ImGuiViewportPlatformData(mainWindow);
        mainViewport->PlatformUserData = data;
    }

    performanceCounterFrequency_ = SDL_GetPerformanceFrequency();
}

UIPlatformBackendSDL::~UIPlatformBackendSDL()
{
    SPDLOG_LOGGER_DEBUG(logger_, "UIPlatformBackendSDL destruction");
}

void UIPlatformBackendSDL::eraseClipboardData()
{
    if (clipboardTextData_)
        SDL_free(clipboardTextData_);
}

const char* UIPlatformBackendSDL::getClipboardData()
{
    clipboardTextData_ = SDL_GetClipboardText();
    return clipboardTextData_;
}

void UIPlatformBackendSDL::newFrame()
{
    ImGuiIO& io = ImGui::GetIO();
    EXPENGINE_ASSERT(
        io.Fonts->IsBuilt(),
        "Font atlas not built! It is generally built by the renderer backend. "
        "Missing call to renderer NewFrame() function?");

    int w, h;
    if (mainWindow_->isMinimized())
        w = h = 0;
    else
        std::tie(w, h) = mainWindow_->getSize();

    auto [displayW, displayH] = mainWindow_->getDrawableSizeInPixels();

    io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2(
            static_cast<float>(displayW) / w, static_cast<float>(displayH / h));

    /* Setup time step. SDL_GetTicks() is not used because it is using millisecond
     * resolution) */
    uint64_t currentTime = SDL_GetPerformanceCounter();
    io.DeltaTime = previousTime_ > 0
        ? (float) ((double) (currentTime - previousTime_) / performanceCounterFrequency_)
        : (float) (1.0f / 60.0f);
    previousTime_ = currentTime;

    UpdateMouseposAndButtons();
    UpdateMouseCursor();
    UpdateGamepads();
}

void UIPlatformBackendSDL::UpdateMouseposAndButtons()
{
    ImGuiIO& io = ImGui::GetIO();
    SDL_Window* window = (SDL_Window*) mainWindow_->getPlatformHandle();

    /* Only when requested by io.WantSetMousePos: set OS mouse pos from Dear ImGui
     * mouse pos. (rarely used, mostly when ImGuiConfigFlags_NavEnableSetMousePos is
     * enabled by user) */
    if (io.WantSetMousePos)
    {
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            SDL_WarpMouseGlobal((int) io.MousePos.x, (int) io.MousePos.y);
        else
#endif
            SDL_WarpMouseInWindow(window, (int) io.MousePos.x, (int) io.MousePos.y);
    }
    else
    {
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    }

    /* Set Dear ImGui mouse pos from OS mouse pos + get buttons. (this is the common
     * behavior) */
    int mouseLocalX, mouseLocalY;
    uint32_t mouseButtons = SDL_GetMouseState(&mouseLocalX, &mouseLocalY);
    /* If a mouse press event came, always pass it as "mouse held this
     * frame", so we don't miss click-release events that are shorter
     * than 1 frame. */
    io.MouseDown[0]
        = mousePressed_[0] || (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    io.MouseDown[1]
        = mousePressed_[1] || (mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2]
        = mousePressed_[2] || (mouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    mousePressed_[0] = mousePressed_[1] = mousePressed_[2] = false;

#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE && !defined(__EMSCRIPTEN__)                    \
    && !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS)
    if (mouseCanUseGlobalState_)
    {
        /* SDL 2.0.4 and later has SDL_GetGlobalMouseState and
         * SDL_CaptureMouse */
        int mouseGlobalX, mouseGlobalY;
        SDL_GetGlobalMouseState(&mouseGlobalX, &mouseGlobalY);

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            /* Multi-viewport mode: mouse position in OS absolute coordinates
             * (io.MousePos is (0,0) when the mouse is on the upper-left of the
             * primary monitor) */
            if (SDL_Window* focusedWindow = SDL_GetKeyboardFocus())
                if (ImGui::FindViewportByPlatformHandle((void*) focusedWindow)
                    != NULL)
                    io.MousePos = ImVec2((float) mouseGlobalX, (float) mouseGlobalY);
        }
        else
        {
            /* Single-viewport mode: mouse position in client window
             * coordinates io.MousePos is (0,0) when the mouse is on the upper-left
             * corner of the app window) */
            if (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)
            {
                int windowX, windowY;
                SDL_GetWindowPosition(window, &windowX, &windowY);
                io.MousePos = ImVec2(
                    (float) (mouseGlobalX - windowX),
                    (float) (mouseGlobalY - windowY));
            }
        }
    }
    else
    {
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)
            io.MousePos = ImVec2((float) mouseLocalX, (float) mouseLocalY);
    }

    /* SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the SDL
     * window boundaries shouldn't e.g. trigger the OS window resize cursor. The
     * function is only supported from SDL 2.0.4 (released Jan 2016) */
    bool any_mouse_button_down = ImGui::IsAnyMouseDown();
    SDL_CaptureMouse(any_mouse_button_down ? SDL_TRUE : SDL_FALSE);
#else
    // SDL 2.0.3 and before: single-viewport only
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS)
        io.MousePos = ImVec2((float) mouseLocalX, (float) mouseLocalY);
#endif
}

void UIPlatformBackendSDL::UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imguiCursor == ImGuiMouseCursor_None)
    {
        /* Hide OS mouse cursor if imgui is drawing it or if it wants no cursor */
        SDL_ShowCursor(SDL_FALSE);
    }
    else
    {
        /* Show OS mouse cursor */
        SDL_SetCursor(
            mouseCursors_[imguiCursor] ? mouseCursors_[imguiCursor]
                                       : mouseCursors_[ImGuiMouseCursor_Arrow]);
        SDL_ShowCursor(SDL_TRUE);
    }
}

void UIPlatformBackendSDL::UpdateGamepads()
{
    ImGuiIO& io = ImGui::GetIO();

    memset(io.NavInputs, 0, sizeof(io.NavInputs));
    if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) == 0)
        return;

    /* Get gamepad */
    SDL_GameController* gameController = SDL_GameControllerOpen(0);
    if (!gameController)
    {
        io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
        return;
    }

/* Update gamepad inputs */
#define MAP_BUTTON(NAV_NO, BUTTON_NO)                                               \
    {                                                                               \
        io.NavInputs[NAV_NO]                                                        \
            = (SDL_GameControllerGetButton(gameController, BUTTON_NO) != 0) ? 1.0f  \
                                                                            : 0.0f; \
    }
#define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1)                                         \
    {                                                                               \
        float vn                                                                    \
            = (float) (SDL_GameControllerGetAxis(gameController, AXIS_NO) - V0)     \
            / (float) (V1 - V0);                                                    \
        if (vn > 1.0f)                                                              \
            vn = 1.0f;                                                              \
        if (vn > 0.0f && io.NavInputs[NAV_NO] < vn)                                 \
            io.NavInputs[NAV_NO] = vn;                                              \
    }
    /* SDL_gamecontroller.h suggests using this value. */
    const int thumbDeadZone = 8000;
    // Cross / A
    MAP_BUTTON(ImGuiNavInput_Activate, SDL_CONTROLLER_BUTTON_A);
    // Circle / B
    MAP_BUTTON(ImGuiNavInput_Cancel, SDL_CONTROLLER_BUTTON_B);
    // Square / X
    MAP_BUTTON(ImGuiNavInput_Menu, SDL_CONTROLLER_BUTTON_X);
    // Triangle / Y
    MAP_BUTTON(ImGuiNavInput_Input, SDL_CONTROLLER_BUTTON_Y);
    // D-Pad Left
    MAP_BUTTON(ImGuiNavInput_DpadLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    // D-Pad Right
    MAP_BUTTON(ImGuiNavInput_DpadRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    ; // D-Pad Up
    MAP_BUTTON(ImGuiNavInput_DpadUp, SDL_CONTROLLER_BUTTON_DPAD_UP)
    // D-Pad Down
    MAP_BUTTON(ImGuiNavInput_DpadDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    // L1 / LB
    MAP_BUTTON(ImGuiNavInput_FocusPrev, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    // R1 / RB
    MAP_BUTTON(ImGuiNavInput_FocusNext, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    // L1 / LB
    MAP_BUTTON(ImGuiNavInput_TweakSlow, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    // R1 / RB
    MAP_BUTTON(ImGuiNavInput_TweakFast, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    MAP_ANALOG(
        ImGuiNavInput_LStickLeft, SDL_CONTROLLER_AXIS_LEFTX, -thumbDeadZone, -32768);
    MAP_ANALOG(
        ImGuiNavInput_LStickRight,
        SDL_CONTROLLER_AXIS_LEFTX,
        +thumbDeadZone,
        +32767);
    MAP_ANALOG(
        ImGuiNavInput_LStickUp, SDL_CONTROLLER_AXIS_LEFTY, -thumbDeadZone, -32767);
    MAP_ANALOG(
        ImGuiNavInput_LStickDown, SDL_CONTROLLER_AXIS_LEFTY, +thumbDeadZone, +32767);

    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
#undef MAP_BUTTON
#undef MAP_ANALOG
}

bool UIPlatformBackendSDL::handleEvent(const SDL_Event& event)
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
            if (ImGuiViewport* viewport = ImGui::FindViewportByPlatformHandle(
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
    auto backend = static_cast<UIPlatformBackendSDL*>(userData);
    backend->eraseClipboardData();
    return backend->getClipboardData();
}

static void ImGui_ImplSDL2_SetClipboardText(void* userData, const char* text)
{
    SDL_SetClipboardText(text);
}

static void ImGui_ImplSDL2_CreateWindow(ImGuiViewport* viewport)
{
    Uint32 sdl_flags = 0;
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

    /* Use the main viewport window to create a new renderer-specific window */
    auto mainViewportData
        = (ImGuiViewportPlatformData*) ImGui::GetMainViewport()->PlatformUserData;
    auto window = mainViewportData->window_->clone(
        (int) viewport->Size.x, (int) viewport->Size.y, "No Title Yet", sdl_flags);
    window->setPosition((int) viewport->Pos.x, (int) viewport->Pos.y);

    auto data = new ImGuiViewportPlatformData(window);
    viewport->PlatformUserData = data;
    viewport->PlatformHandle = data->window_->getPlatformHandle();
    viewport->PlatformHandleRaw = data->window_->getPlatformHandleRaw();
}

static void ImGui_ImplExpengine_DestroyWindow(ImGuiViewport* viewport)
{
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    delete data;
    viewport->PlatformUserData = viewport->PlatformHandle = nullptr;
}

static void ImGui_ImplExpengine_ShowWindow(ImGuiViewport* viewport)
{
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
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

    /* SDL hack: SDL always activate/focus windows */
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
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    auto [x, y] = data->window_->getPosition();
    return ImVec2((float) x, (float) y);
}

static void ImGui_ImplExpengine_SetWindowPos(ImGuiViewport* viewport, ImVec2 pos)
{
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    data->window_->setPosition((int) pos.x, (int) pos.y);
}

static ImVec2 ImGui_ImplExpengine_GetWindowSize(ImGuiViewport* viewport)
{
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    auto [w, h] = data->window_->getPosition();
    return ImVec2((float) w, (float) h);
}

static void ImGui_ImplExpengine_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    data->window_->setSize((int) size.x, (int) size.y);
}

static void ImGui_ImplExpengine_SetWindowTitle(
    ImGuiViewport* viewport,
    const char* title)
{
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    data->window_->setTitle(title);
}

static void ImGui_ImplExpengine_SetWindowAlpha(ImGuiViewport* viewport, float alpha)
{
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    data->window_->setOpacity(alpha);
}

static void ImGui_ImplExpengine_SetWindowFocus(ImGuiViewport* viewport)
{
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    data->window_->setFocus();
}

static bool ImGui_ImplExpengine_GetWindowFocus(ImGuiViewport* viewport)
{
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    return data->window_->isFocused();
}

static bool ImGui_ImplExpengine_GetWindowMinimized(ImGuiViewport* viewport)
{
    auto data = (ImGuiViewportPlatformData*) viewport->PlatformUserData;
    return data->window_->isMinimized();
}

} // namespace render
} // namespace expengine
