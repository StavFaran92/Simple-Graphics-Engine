#include "VolumetricCloudsComponent.h"

#include "Transformation.h"
#include "VolumeComponent.h"
#include "core/Logger.h"

VolumetricCloudsComponent::VolumetricCloudsComponent(Entity entity)
	: entity(entity)
{

}

AssetHandle<MaterialAsset> VolumetricCloudsComponent::getMaterial()
{
	if (!entity.valid())
	{
		logWarning("Invalid Entity set in water body");
		return AssetHandle<MaterialAsset>::empty;
	}

	VolumeComponent& volumeComponent = entity.getComponentInChildren<VolumeComponent>();
	AssetHandle<MaterialAsset>& mat = volumeComponent.material;
	return mat;
}

void VolumetricCloudsComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, ResourceWrapper<Scene>& scene)
{
	if (auto tc = std::dynamic_pointer_cast<VolumetricCloudsComponent>(c))
	{
		tc->entity = entityHandler;
		entityHandler.addComponent<VolumetricCloudsComponent>(*tc);
	}
}
