#include "ExperimEngineConfig.h"

#include "Application.hpp"

#include "Quicktest.hpp"
#include <chrono>
#include <iostream>

#include <random>
#include <thread>

namespace {
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char* APPLICATION_NAME = "ExperimEngine";
const char* WINDOW_TITLE = APPLICATION_NAME;
const float ONE_SEC_IN_MILLI_F = 1000.0f;
} // namespace

int main(int argc, char* argv[])
{
	expengine::Application app;

	try {
		app.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

namespace expengine {

void Application::run()
{
	std::cout << "Application::run\n";

	quicktest::testCMake();
	quicktest::testSol();
	quicktest::testLuaJit();

	initWindow();
	initRenderer();
	mainLoop();
	cleanup();
}

void Application::mainLoop()
{
	while (!(window_->shouldClose())) {
		window_->pollEvents();
		renderFrame();
	}
	renderer_->rendererWaitIdle();
}

void Application::renderFrame()
{
	auto timeStart = std::chrono::high_resolution_clock::now();

	renderer_->render();

	/* Temporary */
	std::mt19937_64 eng { std::random_device {}() };
	std::uniform_int_distribution<> dist { 10, 15 };
	std::this_thread::sleep_for(std::chrono::milliseconds { dist(eng) });

	auto timeEnd = std::chrono::high_resolution_clock::now();
	auto msTimeDiff = std::chrono::duration<double, std::milli>(timeEnd - timeStart).count();

	EngineTimings* timings = &engineParams_.timings;
	timings->frameDuration = (float) msTimeDiff / ONE_SEC_IN_MILLI_F;
	if (!timings->paused) {
		timings->timer += timings->timerSpeed * timings->frameDuration;
		if (timings->timer > 1.0) {
			timings->timer -= 1.0f;
		}
	}

	EngineStatistics* stats = &engineParams_.statistics;
	stats->frameCounter++;
	stats->fpsTimer += (float) msTimeDiff;
	if (stats->fpsTimer > stats->fpsRefreshPeriod) {
		stats->fpsValue = static_cast<uint32_t>((float) stats->frameCounter
												* (ONE_SEC_IN_MILLI_F / stats->fpsRefreshPeriod)
												* (stats->fpsTimer / stats->fpsRefreshPeriod));

		std::cout << "Update FPS value : " << stats->fpsValue
				  << " ; frames : " << stats->frameCounter << " ; timer : " << timings->timer
				  << std::endl;

		stats->fpsTimer = 0.0f;
		stats->frameCounter = 0;
	}
}

void Application::cleanup() {}

void Application::initWindow()
{
	window_ = std::make_unique<render::Window>(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
}

void Application::initRenderer()
{
	renderer_ = std::make_unique<render::Renderer>(APPLICATION_NAME, *window_, engineParams_);
}
} // namespace expengine