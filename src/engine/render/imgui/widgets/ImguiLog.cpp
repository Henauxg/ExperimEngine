#include "ImguiLog.hpp"

namespace expengine {
namespace render {

ImguiLog::ImguiLog(int logCapacity)
	: logCapacity_(logCapacity)
	, autoScroll_(true)
{
	clearAll();
}

void ImguiLog::clearAll()
{
	addedLogs_ = 0;
	bufferIndex_ = 0;
	clear(0);
	clear(1);
}

void ImguiLog::clear(int bufferIndex)
{
	buffers_[bufferIndex].clear();
	lineOffsetVectors_[bufferIndex].clear();
	lineOffsetVectors_[bufferIndex].push_back(0);
}

void ImguiLog::addLog(const char* fmt, ...)
{
	/* Format and append */
	int old_size = buffers_[bufferIndex_].size();
	va_list args;
	va_start(args, fmt);
	buffers_[bufferIndex_].appendfv(fmt, args);
	va_end(args);

	updateLineOffsets(old_size);
	updateBufferSwap();
}

void ImguiLog::addFormattedLog(const char* msg)
{
	/* Msg already formatted */
	int old_size = buffers_[bufferIndex_].size();
	buffers_[bufferIndex_].append(msg);

	updateLineOffsets(old_size);
	updateBufferSwap();
}

void ImguiLog::updateBufferSwap()
{
	addedLogs_++;
	if (addedLogs_ >= logCapacity_)
	{
		swapBuffers();
	}
}

void ImguiLog::updateLineOffsets(int oldSize)
{
	for (int newSize = buffers_[bufferIndex_].size(); oldSize < newSize;
		 oldSize++)
		if (buffers_[bufferIndex_][oldSize] == '\n')
			lineOffsetVectors_[bufferIndex_].push_back(oldSize + 1);
}

void ImguiLog::swapBuffers()
{
	bufferIndex_ = (bufferIndex_ + 1) % 2;
	clear(bufferIndex_);
	addedLogs_ = 0;
}

void ImguiLog::drawBuffer(int bufferIndex)
{
	ImVector<int>& lineOffsets = lineOffsetVectors_[bufferIndex];
	const char* buf = buffers_[bufferIndex].begin();
	const char* buf_end = buffers_[bufferIndex].end();
	if (filter_.IsActive())
	{
		// In this example we don't use the clipper when Filter is enabled.
		// This is because we don't have a random access on the result on
		// our filter. A real application processing logs with ten of
		// thousands of entries may want to store the result of
		// search/filter. especially if the filtering function is not
		// trivial (e.g. reg-exp).
		for (int line_no = 0; line_no < lineOffsets.Size; line_no++)
		{
			const char* line_start = buf + lineOffsets[line_no];
			const char* line_end = (line_no + 1 < lineOffsets.Size)
				? (buf + lineOffsets[line_no + 1] - 1)
				: buf_end;
			if (filter_.PassFilter(line_start, line_end))
				ImGui::TextUnformatted(line_start, line_end);
		}
	}
	else
	{
		// The simplest and easy way to display the entire buffer:
		//   ImGui::TextUnformatted(buf_begin, buf_end);
		// And it'll just work. TextUnformatted() has specialization for
		// large blob of text and will fast-forward to skip non-visible
		// lines. Here we instead demonstrate using the clipper to only
		// process lines that are within the visible area. If you have tens
		// of thousands of items and their processing cost is
		// non-negligible, coarse clipping them on your side is
		// recommended. Using ImGuiListClipper requires A) random access
		// into your data, and B) items all being the  same height, both of
		// which we can handle since we an array pointing to the beginning
		// of each line of text. When using the filter (in the block of
		// code above) we don't have random access into the data to display
		// anymore, which is why we don't use the clipper. Storing or
		// skimming through the search result would make it possible (and
		// would be recommended if you want to search through tens of
		// thousands of entries)
		ImGuiListClipper clipper;
		clipper.Begin(lineOffsets.Size);
		while (clipper.Step())
		{
			for (int line_no = clipper.DisplayStart;
				 line_no < clipper.DisplayEnd; line_no++)
			{
				const char* line_start = buf + lineOffsets[line_no];
				const char* line_end = (line_no + 1 < lineOffsets.Size)
					? (buf + lineOffsets[line_no + 1] - 1)
					: buf_end;
				ImGui::TextUnformatted(line_start, line_end);
			}
		}
		clipper.End();
	}
}

void ImguiLog::draw()
{
	// Options menu
	if (ImGui::BeginPopup("Options"))
	{
		ImGui::Checkbox("Auto-scroll", &autoScroll_);
		ImGui::EndPopup();
	}

	// Main window
	if (ImGui::Button("Options"))
		ImGui::OpenPopup("Options");
	ImGui::SameLine();
	bool shouldClear = ImGui::Button("Clear");
	ImGui::SameLine();
	bool copy = ImGui::Button("Copy");
	ImGui::SameLine();
	filter_.Draw("Filter", -100.0f);

	ImGui::Separator();
	ImGui::BeginChild("scrolling", ImVec2(0, 350), false,
					  ImGuiWindowFlags_HorizontalScrollbar);

	if (shouldClear)
		clearAll();
	if (copy)
		ImGui::LogToClipboard();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	drawBuffer((bufferIndex_ + 1) % 2);
	drawBuffer(bufferIndex_);
	ImGui::PopStyleVar();

	if (autoScroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		ImGui::SetScrollHereY(1.0f);

	ImGui::EndChild();
}

} // namespace render
} // namespace expengine
