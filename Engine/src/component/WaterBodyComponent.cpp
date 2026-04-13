#include "WaterBodyComponent.h"

#include "Transformation.h"
#include "MeshRendererComponent.h"
#include "core/Logger.h"

WaterBodyComponent::WaterBodyComponent(Entity entity)
	: entity(entity)
{

}

AssetHandle<MaterialAsset> WaterBodyComponent::getMaterial()
{
	if (!entity.valid())
	{
		logWarning("Invalid Entity set in water body");
		return AssetHandle<MaterialAsset>::empty;
	}

	MeshRendererComponent& meshRenderer = entity.getComponentInChildren<MeshRendererComponent>(false); // todo fix this is unsafe (getComponentInChildren itself is)
	AssetHandle<MaterialAsset> mat = meshRenderer.getMaterialBySlot(0); // A water body plane only has single material
	return mat;
}

void WaterBodyComponent::resolve(ResourceWrapper<Scene>& scene)
{
	entity.setRegistry(&scene->getRegistry());
}
