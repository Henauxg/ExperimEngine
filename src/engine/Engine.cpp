#include "Engine.hpp"

#include <iostream>

#include <SDL2/SDL.h>
#include <spdlog\sinks\rotating_file_sink.h>
#include <spdlog\sinks\stdout_color_sinks.h>

#include <ExperimEngineConfig.h>

namespace {
const int DEFAULT_WINDOW_WIDTH = 800;
const int DEFAULT_WINDOW_HEIGHT = 600;
const float ONE_SEC_IN_MILLI_F = 1000.0f;
} // namespace

namespace expengine {

Engine::Engine(const char* appName)
{
    /* ------------------------------------------- */
    /* Initialize logging                          */
    /* ------------------------------------------- */
    try
    {
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            LOG_FILE, 1024 * 1024 * 5, 3, true);
        logger_ = std::make_shared<spdlog::logger>(LOGGER_NAME, fileSink);

        /* https://github.com/gabime/spdlog/issues/1318 */
        logger_->set_level(spdlog::level::trace);
        fileSink->set_level(spdlog::level::info);

#ifdef DEBUG
        /* In debug, also redirect logs to standard ouput */
        auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
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

    mainWindow_ = std::make_shared<render::Window>(
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, appName);

    renderer_
        = std::make_unique<render::Renderer>(appName, mainWindow_, engineParams_);

    SPDLOG_LOGGER_INFO(
        logger_,
        "ExperimEngine version : {}.{}.{}",
        ExperimEngine_VERSION_MAJOR,
        ExperimEngine_VERSION_MINOR,
        ExperimEngine_VERSION_PATCH);
}

Engine::~Engine()
{
    SPDLOG_LOGGER_INFO(logger_, "ExperimEngine : cleaning resources");
    SDL_Quit();
}

void Engine::run()
{
    SPDLOG_LOGGER_INFO(logger_, "ExperimEngine : execution start");
    bool shouldStop = false;
    while (!shouldStop)
    {
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
                shouldStop = true;
            }
        }

        render();
    }
    renderer_->rendererWaitIdle();
    SPDLOG_LOGGER_INFO(logger_, "ExperimEngine : execution ended");
}

void Engine::render()
{
    auto timeStart = std::chrono::high_resolution_clock::now();

    renderer_->render();

    auto timeEnd = std::chrono::high_resolution_clock::now();
    auto msTimeDiff
        = std::chrono::duration<double, std::milli>(timeEnd - timeStart).count();

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
            logger_,
            "Update FPS value : {} ; frames : {} ; timer : {}",
            stats->fpsValue,
            stats->frameCounter,
            timings->timer);

        stats->fpsTimer = 0.0f;
        stats->frameCounter = 0;
    }
}

} // namespace expengine
