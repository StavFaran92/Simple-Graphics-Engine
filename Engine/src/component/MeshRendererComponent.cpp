#include "MeshRendererComponent.h"

MeshRendererComponent::MeshRendererComponent(AssetWrapper<MeshCollection> mesh)
	: mesh(mesh)
{
	auto meshCount = mesh.resource()->getMeshes().size();
	for (int i = 0; i < meshCount; i++)
	{
		addMaterial(AssetWrapper<Material>::empty);
	}
}