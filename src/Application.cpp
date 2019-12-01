#include "ExperimEngineConfig.h"

#include "Application.hpp"

#include <iostream>
#include <chrono>
#include "Quicktest.hpp"


#include <thread>
#include <random>

namespace {
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;
	const char* WINDOW_TITILE = "ExperimEngine";
	const float ONE_SEC_IN_MILLI_F = 1000.0f;
}

int main(int argc, char* argv[])
{
	experimengine::Application app;

	try {
		app.run();
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

namespace experimengine {

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
		while (!(window->shouldClose()))
		{
			window->pollEvents();
			renderFrame();
		}
		renderer->rendererWaitIdle();
	}

	void Application::renderFrame()
	{
		auto timeStart = std::chrono::high_resolution_clock::now();

		renderer->render();

		/* Temporary */
		std::mt19937_64 eng{ std::random_device{}() };
		std::uniform_int_distribution<> dist{ 10, 15 };
		std::this_thread::sleep_for(std::chrono::milliseconds{ dist(eng) });

		auto timeEnd = std::chrono::high_resolution_clock::now();
		auto msTimeDiff = std::chrono::duration<double, std::milli>(timeEnd - timeStart).count();

		EngineTimings* timings = &engineParams.timings;
		timings->frameDuration = (float)msTimeDiff / ONE_SEC_IN_MILLI_F;
		if (!timings->paused)
		{
			timings->timer += timings->timerSpeed * timings->frameDuration;
			if (timings->timer > 1.0)
			{
				timings->timer -= 1.0f;
			}
		}

		EngineStatistics* stats = &engineParams.statistics;
		stats->frameCounter++;
		stats->fpsTimer += (float)msTimeDiff;
		if (stats->fpsTimer > stats->fpsRefreshPeriod)
		{
			stats->fpsValue = static_cast<uint32_t>((float)stats->frameCounter * (ONE_SEC_IN_MILLI_F / stats->fpsRefreshPeriod) * (stats->fpsTimer / stats->fpsRefreshPeriod));

			std::cout << "Update FPS value : " << stats->fpsValue << " ; frames : "
				<< stats->frameCounter << " ; timer : "
				<< timings->timer << std::endl;

			stats->fpsTimer = 0.0f;
			stats->frameCounter = 0;
		}
	}

	void Application::cleanup()
	{
		renderer.reset();
		window.reset();
	}

	void Application::initWindow()
	{
		window = std::make_unique<render::Window>(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITILE);
	}

	void Application::initRenderer()
	{
		renderer = std::make_unique<render::Renderer>(*window, engineParams);
	}
}