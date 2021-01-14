#pragma once

#include <memory>

namespace expengine {
class Engine;
}

class Application {
public:
    Application();
    void run();
    void tick();

private:
    std::unique_ptr<expengine::Engine> engine_;
};
