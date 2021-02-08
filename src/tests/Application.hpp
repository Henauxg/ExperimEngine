#pragma once

#include <memory>

namespace experim {
class Engine;
class ImguiLog;
} // namespace experim

union SDL_Event;

class Application {
public:
    Application();
    void run();
    void onEvent(SDL_Event event);
    void onTick(float deltaT);

private:
    std::unique_ptr<experim::Engine> engine_;
    std::unique_ptr<experim::ImguiLog> uiLog_;
};
