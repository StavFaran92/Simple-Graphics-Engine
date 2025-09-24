#include "systems/BuiltInMaterials.h"

BuiltInMaterials::BuiltInMaterials()
{
	AssetInfo aInfo;
	aInfo.isTransient = true;
	aInfo.name = "SGE_MATERIAL_DEFAULT";
	m_defaultMaterial = Material::create(aInfo);
}

ResourceWrapper<Material> BuiltInMaterials::getMaterial(const std::string& material)
{
	throw std::runtime_error("Not yet implemented!");

	return ResourceWrapper<Material>();
}

ResourceWrapper<Material> BuiltInMaterials::getDefaultMaterial() const
{
	return m_defaultMaterial;
}
