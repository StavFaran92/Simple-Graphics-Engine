#include "component/NativeScriptComponent.h"

#include "runtime/Scene.h"

void NativeScriptComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto nc = std::dynamic_pointer_cast<NativeScriptComponent>(c))
	{
		auto& nsc = entityHandler.addComponent<NativeScriptComponent>(*nc);
		nsc.entity.setRegistry(&scene.getRegistry());
		if (nsc.script)
		{
			nsc.script->entity.setRegistry(&scene.getRegistry());
		}
	}
}