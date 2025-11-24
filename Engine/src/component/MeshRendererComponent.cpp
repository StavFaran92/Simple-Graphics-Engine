#include "MeshRendererComponent.h"

#include "memory/BuiltInAssets.h"

MeshRendererComponent::MeshRendererComponent(AssetWrapper<MeshCollection> mesh)
	: mesh(mesh)
{
	auto meshCount = mesh.resource()->getMeshes().size();
	for (int i = 0; i < meshCount; i++)
	{
		addMaterial(BuiltInAssets::getByName<Material>(SGE_MATERIAL_DEFAULT));
	}
}