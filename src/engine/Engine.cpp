#include "Engine.hpp"

#include <iostream>

#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <engine/render/wgpu/WGpuRenderer.hpp>
#else
#include <engine/render/vlk/VlkRenderer.hpp>
#include <spdlog/sinks/rotating_file_sink.h>
#endif

#include <ExperimEngineConfig.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace {

const int DEFAULT_WINDOW_WIDTH = 1280;
const int DEFAULT_WINDOW_HEIGHT = 720;
const float ONE_SEC_IN_MILLI_F = 1000.0f;

} // namespace

namespace expengine {

Engine::Engine(const std::string& appName, const uint32_t appVersion)
{
    /* ------------------------------------------- */
    /* Initialize logging                          */
    /* ------------------------------------------- */

    try
    {
        logger_ = std::make_shared<spdlog::logger>(LOGGER_NAME);
        logger_->set_level(spdlog::level::trace);
        logger_->flush_on(spdlog::level::err);
        /* Globally register the loggers. Accessible with spdlog::get */
        spdlog::register_logger(logger_);
        /* Default accessible as spdlog::info() */
        spdlog::set_default_logger(logger_);

#ifndef __EMSCRIPTEN__
        /* In native, redirect logs to a file */
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            LOG_FILE, 1024 * 1024 * 5, 3, true);
        /* https://github.com/gabime/spdlog/issues/1318 */
        fileSink->set_level(spdlog::level::info);
        logger_->sinks().push_back(fileSink);
#endif // __EMSCRIPTEN__

#ifndef NDEBUG
        /* In debug, also redirect logs to standard ouput */
        auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        stdoutSink->set_level(spdlog::level::trace);
        logger_->sinks().push_back(stdoutSink);
#endif // NDEBUG
    } catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log initialization failed : " << ex.what() << std::endl;
        throw ex;
    }

    /* ------------------------------------------- */
    /* Initialize SDL components                   */
    /* ------------------------------------------- */
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);

    /* ------------------------------------------- */
    /* Initialize main window & renderer           */
    /* ------------------------------------------- */
#ifdef __EMSCRIPTEN__
    renderer_ = std::make_unique<render::webgpu::WebGpuRenderer>(
        appName,
        appVersion,
        DEFAULT_WINDOW_WIDTH,
        DEFAULT_WINDOW_HEIGHT,
        engineParams_);
#else
    renderer_ = std::make_unique<render::vlk::VulkanRenderer>(
        appName,
        appVersion,
        DEFAULT_WINDOW_WIDTH,
        DEFAULT_WINDOW_HEIGHT,
        engineParams_);
#endif

    mainWindow_ = renderer_->getMainWindow();

    SPDLOG_LOGGER_INFO(
        logger_,
        "ExperimEngine : engine created, version : {}.{}.{}",
        ExperimEngine_VERSION_MAJOR,
        ExperimEngine_VERSION_MINOR,
        ExperimEngine_VERSION_PATCH);
}

Engine::~Engine()
{
    SPDLOG_LOGGER_INFO(logger_, "ExperimEngine : cleaning resources");
    SDL_Quit();
}

#ifdef __EMSCRIPTEN__
void emscriptenTick(Engine* engine) { engine->tick(); }
#endif

void Engine::run()
{
    SPDLOG_LOGGER_INFO(logger_, "ExperimEngine : execution start");

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(
        (em_arg_callback_func) emscriptenTick, this, 0, true);
#else
    while (tick()) { }
#endif

    renderer_->waitIdle();
    SPDLOG_LOGGER_INFO(logger_, "ExperimEngine : execution ended");
}

bool Engine::tick()
{
    bool shouldContinue = true;

    /* Events */
    /* TODO : May wrap SDL event */
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        renderer_->handleEvent(event);

        /* SDL_QUIT is only present when the last window is closed.
         * Since we may have multiple windows, we check for our
         * main window close event too. */
        if (event.type == SDL_QUIT
            || (event.type == SDL_WINDOWEVENT
                && event.window.event == SDL_WINDOWEVENT_CLOSE
                && event.window.windowID == mainWindow_->getWindowId()))
        {

            shouldContinue = false;
        }
    }

    render();

    return shouldContinue;
}

void Engine::render()
{
    auto timeStart = std::chrono::steady_clock::now();
    renderer_->render();
    auto timeEnd = std::chrono::steady_clock::now();

    EngineTimings* timings = &engineParams_.timings;
    timings->frameDuration
        = (float) std::chrono::duration_cast<std::chrono::microseconds>(
              timeEnd - timeStart)
              .count()
        / 1000.0F;
    if (!timings->paused)
    {
        timings->timer += timings->timerSpeed * timings->frameDuration / 1000.0F;
        if (timings->timer > 1.0)
        {
            timings->timer -= 1.0f;
        }
    }

    EngineStatistics* stats = &engineParams_.statistics;
    stats->frameCounter++;
    stats->fpsTimer += timings->frameDuration;
    if (stats->fpsTimer > stats->fpsRefreshPeriod)
    {
        stats->fpsValue = static_cast<uint32_t>(
            (float) stats->frameCounter
            * (ONE_SEC_IN_MILLI_F / stats->fpsRefreshPeriod)
            * (stats->fpsTimer / stats->fpsRefreshPeriod));

        SPDLOG_LOGGER_INFO(
            logger_,
            "Update FPS value : {:4} ; frames : {:3} ; timer : {:.5f}, "
            "last frame duration : {:.3f} ms",
            stats->fpsValue,
            stats->frameCounter,
            timings->timer,
            timings->frameDuration);

        stats->fpsTimer = 0.0f;
        stats->frameCounter = 0;
    }
}

} // namespace expengine
