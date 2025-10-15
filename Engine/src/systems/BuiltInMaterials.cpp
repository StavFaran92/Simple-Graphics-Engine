#include "systems/BuiltInMaterials.h"

#include "core/Factory.h"

BuiltInMaterials::BuiltInMaterials()
{
	AssetCreateDescriptor aDesc;
	aDesc.isEngineOwned = true;
	aDesc.name = "SGE_MATERIAL_DEFAULT";
	aDesc.aType = AssetType::MATERIAL;
	m_defaultMaterial = Factory<Material>::createUsingCustomUUID(aDesc.name);
	Engine::get()->getSubSystem<Assets>()->createAsset(m_defaultMaterial, aDesc);
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
