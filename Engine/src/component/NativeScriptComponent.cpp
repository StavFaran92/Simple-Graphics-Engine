#include "component/NativeScriptComponent.h"

#include "runtime/Scene.h"

void NativeScriptComponent::init(SceneResourceRef& scene)
{
	script->entity.setRegistry(&scene->getRegistry());
}
