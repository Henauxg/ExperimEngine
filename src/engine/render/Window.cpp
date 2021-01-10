#include "Window.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <engine/log/ExpengineLog.hpp>

namespace expengine {
namespace render {

Window::Window(int width, int height, const std::string& title, uint32_t flags)
{
    /* Calls SDL_Vulkan_LoadLibrary if flags has SDL_WINDOW_VULKAN */
    sdlWindow_ = SDL_CreateWindow(
        title.data(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        flags);

    /* TODO Better error handling since we will create windows on the fly */
    EXPENGINE_ASSERT(sdlWindow_, "Failed to create an SDL window");
}

Window::~Window()
{
    SPDLOG_DEBUG("SDL Window destruction");
    /* Calls SDL_Vulkan_UnloadLibrary if created with SDL_WINDOW_VULKAN */
    SDL_DestroyWindow(sdlWindow_);
}

std::pair<uint32_t, uint32_t> Window::getDrawableSizeInPixels() const
{
    int w = 0, h = 0;
    SDL_GL_GetDrawableSize(sdlWindow_, &w, &h);
    return {(uint32_t) w, (uint32_t) h};
}

std::shared_ptr<Window> Window::clone(
    int width,
    int height,
    const std::string& title,
    uint32_t flags)
{
    return std::make_shared<Window>(width, height, title, flags);
}

void Window::pollEvents()
{ /* TODO Implement */
}

void Window::waitEvents() const
{ /* TODO Implement */
}

void Window::setOpacity(float opacity) { SDL_SetWindowOpacity(sdlWindow_, opacity); }

void Window::setBordered(bool bordered)
{
    SDL_SetWindowBordered(sdlWindow_, (SDL_bool) bordered);
}

void Window::setSize(int w, int h) { SDL_SetWindowSize(sdlWindow_, w, h); }

void Window::setPosition(int x, int y) { SDL_SetWindowPosition(sdlWindow_, x, y); }

void Window::setTitle(const char* title) { SDL_SetWindowTitle(sdlWindow_, title); }

void Window::setFocus() { SDL_RaiseWindow(sdlWindow_); }

std::pair<int, int> Window::getPosition() const
{
    int x = 0, y = 0;
    SDL_GetWindowPosition(sdlWindow_, &x, &y);
    return {x, y};
}

std::pair<int, int> Window::getSize() const
{
    int w = 0, h = 0;
    SDL_GetWindowSize(sdlWindow_, &w, &h);
    return {w, h};
}

bool Window::isFocused() const
{
    return (SDL_GetWindowFlags(sdlWindow_) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

bool Window::isMinimized() const
{
    return (SDL_GetWindowFlags(sdlWindow_) & SDL_WINDOW_MINIMIZED) != 0;
}

void Window::hide() { SDL_HideWindow(sdlWindow_); }

void Window::show() { SDL_ShowWindow(sdlWindow_); }

uint32_t Window::getWindowId() const { return SDL_GetWindowID(sdlWindow_); }

void* Window::getPlatformHandle() const { return (void*) sdlWindow_; }

void* Window::getPlatformHandleRaw() const
{
#if defined(_WIN32)
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (SDL_GetWindowWMInfo(sdlWindow_, &info))
        return info.info.win.window;
#endif

    /* Else default to SDL handle. Could implement other systems later if
     * getPlatformHandleRaw is used. */
    return (void*) sdlWindow_;
}

} // namespace render
} // namespace expengine
