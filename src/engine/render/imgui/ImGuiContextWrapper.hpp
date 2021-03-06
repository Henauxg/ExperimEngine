#pragma once

#include <memory>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/imgui/lib/imgui.h>

/* RAII wrapper for an ImGui context */
namespace experim {

/** Custom back-end */
class ImGuiContextWrapper {
public:
    inline ImGuiContextWrapper() { ctx_ = ImGui::CreateContext(); };
    /* Also destroy all ImGui viewports from this context */
    inline ~ImGuiContextWrapper()
    {
        SPDLOG_DEBUG("ImGuiContext destruction");
        ImGui::DestroyContext(ctx_);
    };
    inline ImGuiContext* get() { return ctx_; };

private:
    ImGuiContext* ctx_;
};

} // namespace experim
