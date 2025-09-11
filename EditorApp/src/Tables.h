#pragma once

#include "imgui.h"

#define BEGIN_IMGUI_TABLE(name) \
	if (ImGui::BeginTable(name, 2, ImGuiTableFlags_None)) { \
	ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthStretch, 0.4f); \
	ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch, 0.6f);

#define END_IMGUI_TABLE() ImGui::EndTable(); };

static void addTableRow(const std::string& rowName, std::function<void(std::string id)> func)
{
	ImGui::TableNextRow();

	// Key: stick to left (default)
	ImGui::TableSetColumnIndex(0);
	ImGui::TextUnformatted(rowName.c_str());

	// Value: right-align the DragFloat3
	ImGui::TableSetColumnIndex(1);

	const float fullWidth = ImGui::GetColumnWidth();
	const float itemWidth = ImGui::CalcItemWidth(); // or CalcItemWidth(), or a fixed value
	float cursorX = ImGui::GetCursorPosX() + fullWidth - itemWidth;

	ImGui::SetCursorPosX(cursorX);
	ImGui::SetNextItemWidth(itemWidth);
	std::string id = "##" + rowName;
	func(id);
}

static void addTableRowExt(const std::string& rowName, 
	std::function<void(std::string id)> funcKey, 
	std::function<void(std::string id)> funcValue)
{
	ImGui::TableNextRow();

	// Key: stick to left (default)
	ImGui::TableSetColumnIndex(0);
	funcKey(rowName.c_str());

	// Value: right-align the DragFloat3
	ImGui::TableSetColumnIndex(1);

	const float fullWidth = ImGui::GetColumnWidth();
	const float itemWidth = ImGui::CalcItemWidth(); // or CalcItemWidth(), or a fixed value
	float cursorX = ImGui::GetCursorPosX() + fullWidth - itemWidth;

	ImGui::SetCursorPosX(cursorX);
	ImGui::SetNextItemWidth(itemWidth);
	std::string id = "##" + rowName;
	funcValue(id);
} 

static void rightAlignedText(const std::string& text) 
{
	float textWidth = ImGui::CalcTextSize(text.c_str()).x;
	float fullWidth = ImGui::GetColumnWidth();
	float padding = ImGui::GetStyle().ItemSpacing.x;

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + fullWidth - textWidth - padding);
	ImGui::TextUnformatted(text.c_str());
}