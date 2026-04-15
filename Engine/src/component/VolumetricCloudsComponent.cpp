#include "VolumetricCloudsComponent.h"

#include "Transformation.h"
#include "VolumeComponent.h"
#include "core/Logger.h"

VolumetricCloudsComponent::VolumetricCloudsComponent(Entity entity)
	: entity(entity)
{

}

MaterialAssetRef VolumetricCloudsComponent::getMaterial()
{
	if (!entity.valid())
	{
		logWarning("Invalid Entity set in water body");
		return MaterialAssetRef::empty;
	}

	VolumeComponent& volumeComponent = entity.getComponentInChildren<VolumeComponent>();
	MaterialAssetRef& mat = volumeComponent.material;
	return mat;
}