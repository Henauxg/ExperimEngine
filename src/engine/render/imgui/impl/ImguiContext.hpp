#pragma once

#include <memory>

#include <engine/render/imgui/lib/imgui.h>

/* RAII wrapper for an ImGui context */
namespace expengine {
namespace render {

/** Custom back-end */
class ImguiContext {
public:
	inline ImguiContext::ImguiContext() { ctx_ = ImGui::CreateContext(); };
	inline ~ImguiContext() { ImGui::DestroyContext(ctx_); };
	inline ImGuiContext* get() { return ctx_; };

private:
	ImGuiContext* ctx_;
};

} // namespace render
} // namespace expengine
