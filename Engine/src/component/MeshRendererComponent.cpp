#include "MeshRendererComponent.h"

#include "memory/BuiltInAssets.h"
#include "core/Logger.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"

MeshRendererComponent::MeshRendererComponent(AssetHandle<ModelAsset> mesh)
	: mesh(mesh)
{
	int materialCount = mesh.resource()->getMaterialCount();
	for (int i = 0; i < materialCount; i++)
	{
		addMaterial(BuiltInAssets::getByName<MaterialAsset>(SGE_MATERIAL_DEFAULT));
	}
}

AssetHandle<MaterialAsset> MeshRendererComponent::getMaterialBySlot(int slot) const
{
	auto iter = m_material.find(slot);
	if (iter == m_material.end())
	{
		logWarning("Could not find material in slot {}", std::to_string(slot));
		return AssetHandle<MaterialAsset>::empty;
	}
	return iter->second;
}

std::vector<AssetHandle<Asset>*> MeshRendererComponent::gatherDependenciesInternal() const
{
	std::vector<AssetHandle<Asset>*> dependencies;
	dependencies.push_back((AssetHandle<Asset>*) &mesh);
	for (auto& [name, mat] : m_material)
	{
		dependencies.push_back((AssetHandle<Asset>*) &mat);
		auto samplers = mat->getSamplers();
		for (const auto& [sName, sampler] : samplers)
		{
			dependencies.push_back((AssetHandle<Asset>*) &sampler->texture);
		}
	}
	return dependencies;
}