#include "component/ScriptComponent.h"

#include "scripts/ScriptSystem.h"

ScriptComponent::ScriptComponent(const std::string& path)
	: filepath(path)
{

}

void ScriptComponent::loadScript()
{
	if (filepath.empty())
		return;

	Engine::get()->getSubSystem<ScriptSystem>()->loadScript(*this);
}

bool ScriptComponent::isValid() const
{
	return !filepath.empty();
}