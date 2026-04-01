#include "component/ObjectComponent.h"

#include "runtime/Scene.h"

void ObjectComponent::resolve(ResourceWrapper<Scene>& scene)
{
	e.setRegistry(&scene->getRegistry());
}
