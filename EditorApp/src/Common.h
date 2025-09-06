#pragma once

#include "sge.h"
#include "imgui.h"

#include <unordered_map>
#include <string>

inline const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus |
											ImGuiWindowFlags_NoCollapse |
											ImGuiWindowFlags_NoFocusOnAppearing |
											ImGuiWindowFlags_NoTitleBar |
											ImGuiWindowFlags_NoScrollbar |
											ImGuiWindowFlags_NoScrollWithMouse |
											ImGuiWindowFlags_NoMove;

extern std::unordered_map<std::string, Resource<Texture>> icons;