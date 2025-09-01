#include "systems/BuiltInMaterials.h"

BuiltInMaterials::BuiltInMaterials()
{
	m_defaultMaterial = Material::create(true);
}

Resource<Material> BuiltInMaterials::getMaterial(const std::string& material)
{
	throw std::runtime_error("Not yet implemented!");

	return Resource<Material>();
}

Resource<Material> BuiltInMaterials::getDefaultMaterial() const
{
	return m_defaultMaterial;
}
