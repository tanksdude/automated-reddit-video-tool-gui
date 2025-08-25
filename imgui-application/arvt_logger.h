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

	void AddLog(const char* fmt, ...) {
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
	}

	void Draw() {
		/*
		// Options menu
		if (ImGui::BeginPopup("Options")) {
			//ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::EndPopup();
		}

		// Main window
		if (ImGui::Button("Options")) {
			ImGui::OpenPopup("Options");
		}
		ImGui::SameLine();
		bool clear = ImGui::Button("Clear");
		*/

		ImGui::Separator();

		if (ImGui::BeginChild("scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar)) {
			/*
			if (clear) {
				Clear();
			}
			*/

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::TextUnformatted(Buf.begin(), Buf.end());
			ImGui::PopStyleVar();

			// Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
			// Using a scrollbar or mouse-wheel will take away from the bottom edge.
			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
				ImGui::SetScrollHereY(1.0f);
			}
		}
		ImGui::EndChild();
	}
};
