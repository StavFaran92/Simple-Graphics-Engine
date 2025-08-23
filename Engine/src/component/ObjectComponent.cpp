#include "component/ObjectComponent.h"

#include "runtime/Scene.h"

void ObjectComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto oc = std::dynamic_pointer_cast<ObjectComponent>(c))
	{
		auto& obj = entityHandler.addComponent<ObjectComponent>(*oc);
		obj.e.setRegistry(&scene.getRegistry());
	}
}