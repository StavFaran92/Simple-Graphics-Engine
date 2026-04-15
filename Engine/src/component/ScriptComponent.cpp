#include "component/ScriptComponent.h"

#include "scripts/ScriptSystem.h"
#include "runtime/Scene.h"
#include "core/Engine.h"

ScriptComponent::ScriptComponent(const LuaScriptAssetRef& script)
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

LuaScriptAssetRef& ScriptComponent::getScript()
{
	return script;
}

void ScriptComponent::resolve(SceneResourceRef& scene)
{
	entity.setRegistry(&scene->getRegistry());
}
