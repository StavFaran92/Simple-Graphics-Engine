#include "component/ScriptComponent.h"

#include "scripts/ScriptSystem.h"

ScriptComponent::ScriptComponent(const ResourceWrapper<LuaScript>& script)
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

ResourceWrapper<LuaScript>& ScriptComponent::getScript()
{
	return script;
}
