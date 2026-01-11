#include "TextureDataWidget.h"

#include "imgui.h"

void TextureDataWidget::draw()
{
	static const char* semanticNames[] = {
		"Color (Albedo)",
		"Normal Map",
		"Heightmap",
		"Mask",
		"Data / Compute",
		"Environment (HDR)",
		"Lookup Table"
	};

	ImGui::Combo("Texture Usage", (int*)&m_semantic, semanticNames, IM_ARRAYSIZE(semanticNames));

	// --- Filter type ---
	static const char* filterModes[] = {
		"Nearest",
		"Linear"
	};

	ImGui::Text("Filter Mode");
	ImGui::Combo("##FilterMode", (int*)&m_filterMode, filterModes, IM_ARRAYSIZE(filterModes));

	// --- Anisotropy ---
	ImGui::Text("Anisotropy");
	ImGui::SliderFloat("##Aniso", &m_anisotropy, 1.0f, 16.0f);

	// --- Wrap mode (the one you meant: repeat / clamp-to-edge / clamp-to-border / mirrored-repeat) ---
	static const char* wrapModes[] = {
		"Repeat",
		"Clamp",
		"Mirror"
	};

	ImGui::Text("Wrap Mode");
	ImGui::Combo("##WrapMode", (int*)&m_wrapMode, wrapModes, IM_ARRAYSIZE(wrapModes));
}

