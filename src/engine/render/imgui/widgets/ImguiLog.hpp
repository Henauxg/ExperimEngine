#pragma once

#include <engine/render/imgui/lib/imgui.h>

namespace expengine {
namespace render {

/** Inspired by ImGui log example in the demo window, but with a maximal
 * buffer capacity and double-buffered to keep recent history when max
 * capacity (of 1 buffer) is reached */
class ImguiLog {
public:
	/* logCapacity for 1 buffer */
	ImguiLog(int logCapacity = DEFAULT_CAPACITY);
	void clearAll();
	void addLog(const char* fmt, ...);
	void addFormattedLog(const char* msg);
	void draw();

private:
	enum { DEFAULT_CAPACITY = 500 };
	void swapBuffers();
	void clear(int bufferIndex);
	void drawBuffer(int bufferIndex);

	inline void updateBufferSwap();
	inline void updateLineOffsets(int oldSize);

	ImGuiTextBuffer buffers_[2];
	ImVector<int> lineOffsetVectors_[2];
	int bufferIndex_;
	int addedLogs_;
	int logCapacity_;
	bool autoScroll_;
	ImGuiTextFilter filter_;
};

} // namespace render
} // namespace expengine