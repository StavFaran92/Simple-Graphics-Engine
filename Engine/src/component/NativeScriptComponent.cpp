#include "component/NativeScriptComponent.h"

#include "runtime/Scene.h"

void NativeScriptComponent::resolve(SceneResourceRef& scene)
{
	script->entity.setRegistry(&scene->getRegistry());
}
