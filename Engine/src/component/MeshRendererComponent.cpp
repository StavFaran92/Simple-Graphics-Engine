#include "MeshRendererComponent.h"

#include "memory/BuiltInAssets.h"

MeshRendererComponent::MeshRendererComponent(AssetHandle<MeshGroupAsset> mesh)
	: mesh(mesh)
{
	int materialCount = mesh.get()->getMaterialCount();
	for (int i = 0; i < materialCount; i++)
	{
		addMaterial(BuiltInAssets::getByName<Material>(SGE_MATERIAL_DEFAULT));
	}
}

AssetHandle<Material> MeshRendererComponent::getMaterialBySlot(int slot) const
{
	auto iter = m_material.find(slot);
	if (iter == m_material.end())
	{
		logWarning("Could not find material in slot {}", std::to_string(slot));
		return AssetHandle<Material>::empty;
	}
	return iter->second;
}
