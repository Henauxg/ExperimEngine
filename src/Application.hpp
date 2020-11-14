#pragma once

#include <memory>

#include <engine/Engine.hpp>

class Application {
public:
	Application();
	void run();

private:
	std::unique_ptr<expengine::Engine> engine_;
};
