#pragma once

#include <memory>

#include <engine/log/ExpengineLog.hpp>
#include <engine/render/imgui/lib/imgui.h>

/* RAII wrapper for an ImGui context */
namespace expengine {
namespace render {

/** Custom back-end */
class ImguiContext {
public:
	inline ImguiContext() { ctx_ = ImGui::CreateContext(); };
	/* Also destroy all ImGui viewports from this context */
	inline ~ImguiContext()
	{
		SPDLOG_DEBUG("ImguiContext destruction");
		ImGui::DestroyContext(ctx_);
	};
	inline ImGuiContext* get() { return ctx_; };

private:
	ImGuiContext* ctx_;
};

} // namespace render
} // namespace expengine
