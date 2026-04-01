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

void ScriptComponent::resolve(ResourceWrapper<Scene>& scene)
{
	entity.setRegistry(&scene->getRegistry());
}
