#pragma once

#include <memory>
#include <string>

struct SDL_Window;

namespace expengine {
namespace render {

class Window {
public:
    Window(int width, int height, const std::string& title, uint32_t flags);
    ~Window();

    void pollEvents();
    void waitEvents() const;
    void setOpacity(float opacity);
    void setBordered(bool bordered);
    void setSize(int w, int h);
    void setPosition(int x, int y);
    void setTitle(const char* title);
    void setFocus();
    std::pair<int, int> getPosition() const;
    std::pair<int, int> getSize() const;
    bool isFocused() const;
    bool isMinimized() const;
    void hide();
    void show();
    uint32_t getWindowId() const;

    /* Virtual methods */
    virtual std::shared_ptr<Window> clone(
        int width,
        int height,
        const std::string& title,
        uint32_t flags);

    /* Platform/OS specific */
    void* getPlatformHandle() const;
    void* getPlatformHandleRaw() const;

protected:
    struct SDL_Window* sdlWindow_;
};

} // namespace render
} // namespace expengine
