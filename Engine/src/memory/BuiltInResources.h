#pragma once

#include <string>
#include <unordered_map>

#include "core/Configurations.h"
#include "systems/SubSystem.h"
#include "core/Logger.h"
#include "core/Engine.h"

#include "memory/ResourceWrapper.h"

const std::string SGE_RESOURCE_SHADER_DEFFERED_PBR_GEOM = "SGE_RESOURCE_SHADER_DEFFERED_PBR_GEOM";
const std::string SGE_RESOURCE_SHADER_DEFFERED_PBR_LIGHT = "SGE_RESOURCE_SHADER_DEFFERED_PBR_LIGHT";
const std::string SGE_RESOURCE_SHADER_FORWARD_PBR = "SGE_RESOURCE_SHADER_FORWARD_PBR";
const std::string SGE_RESOURCE_SHADER_TERRAIN = "SGE_RESOURCE_SHADER_TERRAIN";
const std::string SGE_RESOURCE_SHADER_VOLUME = "SGE_RESOURCE_SHADER_VOLUME";
const std::string SGE_RESOURCE_SHADER_DEBUG_DATA = "SGE_RESOURCE_SHADER_DEBUG_DATA";
const std::string SGE_RESOURCE_SHADER_FOLIAGE = "SGE_RESOURCE_SHADER_FOLIAGE";
const std::string SGE_RESOURCE_SHADER_TEXTURE_BRUSH_DEFORM = "SGE_RESOURCE_SHADER_TEXTURE_BRUSH_DEFORM";

// This registry represents content that is:
// - Owned and defined by the engine
// - Used as internal defaults or implementation resources
// - Not serialized or referenced by save files
// - Identified by stable, engine-defined IDs
// - Not exposed in the asset browser
class BuiltInResources : public SubSystem
{
public:
	BuiltInResources();

	template<typename T>
	ResourceWrapper<T> getInner(const std::string& name)
	{
		auto it = m_resources.find(name);
		if (it == m_resources.end())
		{
			logError("Could not find resource {}" , name);
			return ResourceWrapper<T>::empty;
		}

		return it->second.as<T>();
	}

	template<typename T>
	static ResourceWrapper<T> get(const std::string& name)
	{
		return Engine::get()->getSubSystem<BuiltInResources>()->getInner<T>(name);
	}

	void loadAllResources();

private:
	std::unordered_map<std::string, ResourceWrapper<Resource>> m_resources;
};