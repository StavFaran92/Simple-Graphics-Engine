#pragma once

#include <map>
#include "memory/Resource.h"
#include "render/Material.h"

class EngineAPI BuiltInMaterials
{
public:
	BuiltInMaterials();
	~BuiltInMaterials() = default;

	Resource<Material> getMaterial(const std::string& material);
	Resource<Material> getDefaultMaterial() const;
private:
	std::map<std::string, Resource<Material>> m_materials;

	Resource<Material> m_defaultMaterial;
};