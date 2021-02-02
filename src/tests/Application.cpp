#include "Application.hpp"

#include <iostream>

#include <engine/Engine.hpp>
#include <engine/log/ExpengineLog.hpp>
#include <engine/render/imgui/lib/imgui.h>
#include <engine/utils/Utils.hpp>

#ifndef __EMSCRIPTEN__
#include <tests/lua/Quicktest.hpp>
#endif // !__EMSCRIPTEN__

/* File private constants */
namespace {

const std::string APPLICATION_NAME = "ExperimEngineApplication";
const uint32_t APPLICATION_VERSION = EXPENGINE_MAKE_VERSION(1, 0, 0);

} // namespace

int main(int argc, char* argv[])
{
    try
    {
        Application app;
        app.run();
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

Application::Application()
{
    engine_
        = std::make_unique<expengine::Engine>(APPLICATION_NAME, APPLICATION_VERSION);
    engine_->onTick(this);
}

void Application::run()
{
#ifndef __EMSCRIPTEN__
    expengine::quicktest::testSol();
    expengine::quicktest::testLuaJit();
#endif // !__EMSCRIPTEN__

    engine_->run();
}

void Application::tick()
{
    static bool show_demo_window = true;
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

#if 1
    ImGui::SetNextWindowBgAlpha(0.2f);
    ImGui::Begin("SimpleWindow 0.2");

    ImGui::Text("Hello, world %d", 123);
    if (ImGui::Button("Log"))
        SPDLOG_INFO("Log");
    static float f = 0;
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

    ImGui::End();

    ImGui::SetNextWindowBgAlpha(0.5f);
    ImGui::Begin("SimpleWindow 0.5");

    ImGui::Text("Hello, world %d", 123);
    if (ImGui::Button("Log"))
        SPDLOG_INFO("Log");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

    ImGui::End();

    ImGui::SetNextWindowBgAlpha(0.8f);
    ImGui::Begin("SimpleWindow 0.8");

    ImGui::Text("Hello, world %d", 123);
    if (ImGui::Button("Log"))
        SPDLOG_INFO("Log");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

    ImGui::End();
#endif
}
