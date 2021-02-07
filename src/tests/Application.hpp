#pragma once

#include <memory>

namespace experim {
class Engine;
}

class Application {
public:
    Application();
    void run();
    void tick(float deltaT);

private:
    std::unique_ptr<experim::Engine> engine_;
};
