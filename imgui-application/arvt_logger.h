#pragma once
#include <string>
#include <imgui.h>

//see IMGUI_DEMO_MARKER("Examples/Log") for the base code
struct ARVT_Logger {
	ImGuiTextBuffer Buf;
	bool AutoScroll;

	ARVT_Logger(bool scrollToBottom) {
		Clear();
		AutoScroll = scrollToBottom;
	}

	void Clear() {
		Buf.clear();
	}

	void Clear_OneLine() {
		int idx = Buf.Buf.find_index('\n');
		if (idx >= 0) {
			Buf.Buf.erase(Buf.Buf.begin(), Buf.Buf.begin() + idx + 1);
		} else {
			Buf.clear();
		}
	}

	void AddLogSpecific(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
	}

	void AddLog(const char* level, const char* type, const char* msg) {
		AddLogSpecific("[%06.2fs] %s %s: %s\n", ImGui::GetTime(), level, type, msg);
	}
	void AddLog(const char* level, const char* type) {
		AddLogSpecific("[%06.2fs] %s %s\n",     ImGui::GetTime(), level, type);
	}

	void Draw() {
		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::TextUnformatted(Buf.begin(), Buf.end());
			ImGui::PopStyleVar();

			// Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
			// Using a scrollbar or mouse-wheel will take away from the bottom edge.
			if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
				ImGui::SetScrollHereY(1.0f);
			}
		}
		ImGui::EndChild(); //must EndChild() regardless of BeginChild()'s return value
	}

	void Draw_Extras() {
		if (ImGui::Button("Clear 1")) {
			Clear_OneLine();
		}
		ImGui::SameLine();
		if (ImGui::Button("Test 1")) {
			AddLog("[info]", "Test", "Test log entry");
		}
		if (ImGui::Button("Clear 10")) {
			//be lazy because I don't care
			for (int i = 0; i < 10; i++) {
				Clear_OneLine();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear All")) {
			Clear();
		}
	}
};
