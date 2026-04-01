#include "component/NativeScriptComponent.h"

#include "runtime/Scene.h"

void NativeScriptComponent::resolve(ResourceWrapper<Scene>& scene)
{
	script->entity.setRegistry(&scene->getRegistry());
}
