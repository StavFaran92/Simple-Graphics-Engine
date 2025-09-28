#include "systems/BuiltInMaterials.h"

#include "core/Factory.h"

BuiltInMaterials::BuiltInMaterials()
{
	AssetDescriptor aDesc;
	aDesc.isTransient = true;
	aDesc.name = "SGE_MATERIAL_DEFAULT";
	aDesc.aType = AssetType::MATERIAL;
	m_defaultMaterial = Factory<Material>::createUsingCustomUUID(aDesc.name);
	Engine::get()->getSubSystem<Assets>()->addAsset(aDesc.parse());
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
