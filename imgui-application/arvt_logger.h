#pragma once
#include <string>
#include "imgui/imgui.h"

//see IMGUI_DEMO_MARKER("Examples/Log") for the base code
struct ARVT_Logger {
	ImGuiTextBuffer Buf;

	ARVT_Logger() {
		Clear();
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

	void AddLog(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
	}

	void Draw() {
		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar)) {
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::TextUnformatted(Buf.begin(), Buf.end());
			ImGui::PopStyleVar();

			// Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
			// Using a scrollbar or mouse-wheel will take away from the bottom edge.
			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
				ImGui::SetScrollHereY(1.0f);
			}
		}
		ImGui::EndChild(); //must EndChild() regardless of BeginChild()'s return value
	}

	void Draw_Extras() {
		// Options menu
		if (ImGui::BeginPopup("Options")) {
			//ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::EndPopup();
		}

		// Main window
		if (ImGui::Button("Clear 1")) {
			Clear_OneLine();
		}
		ImGui::SameLine();
		ImGui::BeginDisabled();
		if (ImGui::Button("Options")) {
			ImGui::OpenPopup("Options");
		}
		ImGui::EndDisabled();
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
