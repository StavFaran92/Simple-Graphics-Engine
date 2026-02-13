#include "component/ScriptComponent.h"

#include "scripts/ScriptSystem.h"
#include "runtime/Scene.h"

ScriptComponent::ScriptComponent(const AssetHandle<LuaScriptAsset>& script)
	: script(script)
{

}

void ScriptComponent::loadScript()
{
	if (!isValid())
		return;

	Engine::get()->getSubSystem<ScriptSystem>()->loadScript(*this);
}

bool ScriptComponent::isValid() const
{
	return !script.isEmpty();
}

AssetHandle<LuaScriptAsset>& ScriptComponent::getScript()
{
	return script;
}

void ScriptComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto tc = std::dynamic_pointer_cast<ScriptComponent>(c))
	{
		auto& script = entityHandler.addComponent<ScriptComponent>(*tc);
		script.entity.setRegistry(&scene.getRegistry());
	}
}
