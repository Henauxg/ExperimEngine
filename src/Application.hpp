#pragma once

#include <memory>

namespace expengine {
class Engine;
}

class Application {
public:
    Application();
    void run();

private:
    std::unique_ptr<expengine::Engine> engine_;
};
