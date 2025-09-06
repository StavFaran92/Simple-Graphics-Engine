#pragma once

#include "sge.h"
#include "imgui.h"

#include <unordered_map>
#include <string>
#include <vector>

inline const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBringToFrontOnFocus |
											ImGuiWindowFlags_NoCollapse |
											ImGuiWindowFlags_NoFocusOnAppearing |
											ImGuiWindowFlags_NoTitleBar |
											ImGuiWindowFlags_NoScrollbar |
											ImGuiWindowFlags_NoScrollWithMouse |
											ImGuiWindowFlags_NoMove;

extern std::unordered_map<std::string, Resource<Texture>> icons;

struct SceneObject 
{
	std::string name;
	Entity e;
};

extern std::vector<SceneObject> sceneObjects;

static void updateScene()
{
	sceneObjects.clear();
	for (auto&& [entity, obj] : Engine::get()->getContext()->getActiveScene()->getRegistry().get().view<ObjectComponent>().each())
	{
		sceneObjects.emplace_back(SceneObject{ obj.name, obj.e });
	}
}