#include "component/VolumeComponent.h"

void VolumeComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, ResourceWrapper<Scene>&)
{
	attachSimple<VolumeComponent>(c, entityHandler);
}
