#include "VolumetricCloudsComponent.h"

#include "Transformation.h"
#include "VolumeComponent.h"

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

	VolumeComponent& volumeComponent = entity.getComponentInChildren<VolumeComponent>();
	AssetWrapper<Material>& mat = volumeComponent.material;
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
