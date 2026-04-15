#include "WaterBodyComponent.h"

#include "Transformation.h"
#include "MeshRendererComponent.h"
#include "core/Logger.h"

WaterBodyComponent::WaterBodyComponent(Entity entity)
	: entity(entity)
{

}

MaterialAssetRef WaterBodyComponent::getMaterial()
{
	if (!entity.valid())
	{
		logWarning("Invalid Entity set in water body");
		return MaterialAssetRef::empty;
	}

	MeshRendererComponent& meshRenderer = entity.getComponentInChildren<MeshRendererComponent>(false); // todo fix this is unsafe (getComponentInChildren itself is)
	MaterialAssetRef mat = meshRenderer.getMaterialBySlot(0); // A water body plane only has single material
	return mat;
}

void WaterBodyComponent::resolve(SceneResourceRef& scene)
{
	entity.setRegistry(&scene->getRegistry());
}
