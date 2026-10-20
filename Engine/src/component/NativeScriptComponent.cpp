#include "component/NativeScriptComponent.h"

#include "runtime/Scene.h"

void NativeScriptComponent::init(SceneResourceRef& scene)
{
	// Unbound (added without bind<T>()); Scene::update warns about it
	if (!script)
		return;

	script->entity.setRegistry(&scene->getRegistry());
}
