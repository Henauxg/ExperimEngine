#pragma once

#include <memory>

namespace experim {
class Engine;
}

class Application {
public:
    Application();
    void run();
    void tick();

private:
    std::unique_ptr<experim::Engine> engine_;
};
