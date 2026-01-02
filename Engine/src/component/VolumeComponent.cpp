#include "component/VolumeComponent.h"

void VolumeComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene&)
{
	attachSimple<VolumeComponent>(c, entityHandler);
}
