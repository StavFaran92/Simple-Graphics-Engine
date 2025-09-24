#pragma once

#include <map>
#include "memory/ResourceWrapper.h"
#include "render/Material.h"

class EngineAPI BuiltInMaterials
{
public:
	BuiltInMaterials();
	~BuiltInMaterials() = default;

	ResourceWrapper<Material> getMaterial(const std::string& material);
	ResourceWrapper<Material> getDefaultMaterial() const;
private:
	std::map<std::string, ResourceWrapper<Material>> m_materials;

	ResourceWrapper<Material> m_defaultMaterial;
};