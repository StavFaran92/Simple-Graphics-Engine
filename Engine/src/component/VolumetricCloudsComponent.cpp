#include "VolumetricCloudsComponent.h"

#include "Transformation.h"
#include "MeshRendererComponent.h"

VolumetricCloudsComponent::VolumetricCloudsComponent(Entity entity)
	: entity(entity)
{

}

AssetWrapper<Material> VolumetricCloudsComponent::getMaterial()
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

void VolumetricCloudsComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto tc = std::dynamic_pointer_cast<VolumetricCloudsComponent>(c))
	{
		tc->entity = entityHandler;
		entityHandler.addComponent<VolumetricCloudsComponent>(*tc);
	}
}
