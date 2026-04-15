#include "component/ObjectComponent.h"

#include "runtime/Scene.h"

void ObjectComponent::resolve(SceneResourceRef& scene)
{
	e.setRegistry(&scene->getRegistry());
}
