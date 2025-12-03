#include "WaterBodyComponent.h"

#include "Transformation.h"
#include "MeshRendererComponent.h"

WaterBodyComponent::WaterBodyComponent(Entity entity)
	: entity(entity)
{

}

AssetWrapper<Material> WaterBodyComponent::getMaterial()
{
	if (!entity.valid())
	{
		logWarning("Invalid Entity set in water body");
		return AssetWrapper<Material>::empty;
	}

	MeshRendererComponent& meshRenderer = entity.getComponentInChildren<MeshRendererComponent>(false); // todo fix this is unsafe (getComponentInChildren itself is)
	AssetWrapper<Material> mat = meshRenderer.getMaterialBySlot(0); // A water body plane only has single material
	return mat;
}
