#include "component/ScriptComponent.h"

#include "scripts/ScriptSystem.h"

void ScriptComponent::loadScript(const std::string& path)
{
	Engine::get()->getSubSystem<ScriptSystem>()->loadScript(path, *this);
}

bool ScriptComponent::isValid() const
{
	return handlerID != -1;
}