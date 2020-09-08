#include "Application.hpp"

#include <chrono>
#include <iostream>
#include <random>
#include <thread>

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <ExperimEngineConfig.h>
#include <test/Quicktest.hpp>

/* File private constants */
namespace {

const int DEFAULT_WINDOW_WIDTH = 800;
const int DEFAULT_WINDOW_HEIGHT = 600;
const char* APPLICATION_NAME = "ExperimEngine";
const char* WINDOW_TITLE = APPLICATION_NAME;
const float ONE_SEC_IN_MILLI_F = 1000.0f;

} // namespace

int main(int argc, char* argv[])
{
	expengine::Application app;

	try
	{
		app.run();
	} catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

namespace expengine {

Application::Application()
{
	/* ------------------------------------------- */
	/* Initialize logging                          */
	/* ------------------------------------------- */
	try
	{
		auto fileSink
			= std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
				LOG_FILE, 1024 * 1024 * 5, 3, true);
		logger_ = std::make_shared<spdlog::logger>(LOGGER_NAME, fileSink);

		/* https://github.com/gabime/spdlog/issues/1318 */
		logger_->set_level(spdlog::level::trace);
		fileSink->set_level(spdlog::level::info);

#ifdef DEBUG
		/* In debug, also redirect logs to standard ouput */
		auto stdoutSink
			= std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		stdoutSink->set_level(spdlog::level::trace);
		logger_->sinks().push_back(stdoutSink);
#endif // DEBUG

		logger_->flush_on(spdlog::level::err);

		/* Globally register the loggers. Accessible with spdlog::get */
		spdlog::register_logger(logger_);

		/* Default accessible as spdlog::info() */
		spdlog::set_default_logger(logger_);

	} catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log initialization failed : " << ex.what()
				  << std::endl;
		// TODO Abort ?
	}

	/* ------------------------------------------- */
	/* Initialize main window & renderer           */
	/* ------------------------------------------- */
	window_ = std::make_unique<render::Window>(
		DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, WINDOW_TITLE);
	renderer_ = std::make_unique<render::Renderer>(
		APPLICATION_NAME, *window_, engineParams_);

	SPDLOG_LOGGER_INFO(logger_, "ExperimEngine version : {}.{}.{}",
					   ExperimEngine_VERSION_MAJOR,
					   ExperimEngine_VERSION_MINOR,
					   ExperimEngine_VERSION_PATCH);
}

void Application::run()
{
	quicktest::testSol();
	quicktest::testLuaJit();

	mainLoop();
	cleanup();
}

void Application::mainLoop()
{
	while (!(window_->shouldClose()))
	{
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
	auto msTimeDiff
		= std::chrono::duration<double, std::milli>(timeEnd - timeStart)
			  .count();

	EngineTimings* timings = &engineParams_.timings;
	timings->frameDuration = (float) msTimeDiff / ONE_SEC_IN_MILLI_F;
	if (!timings->paused)
	{
		timings->timer += timings->timerSpeed * timings->frameDuration;
		if (timings->timer > 1.0)
		{
			timings->timer -= 1.0f;
		}
	}

	EngineStatistics* stats = &engineParams_.statistics;
	stats->frameCounter++;
	stats->fpsTimer += (float) msTimeDiff;
	if (stats->fpsTimer > stats->fpsRefreshPeriod)
	{
		stats->fpsValue = static_cast<uint32_t>(
			(float) stats->frameCounter
			* (ONE_SEC_IN_MILLI_F / stats->fpsRefreshPeriod)
			* (stats->fpsTimer / stats->fpsRefreshPeriod));

		SPDLOG_LOGGER_INFO(
			logger_, "Update FPS value : {} ; frames : {} ; timer : {}",
			stats->fpsValue, stats->frameCounter, timings->timer);

		stats->fpsTimer = 0.0f;
		stats->frameCounter = 0;
	}
}

void Application::cleanup() { }

} // namespace expengine