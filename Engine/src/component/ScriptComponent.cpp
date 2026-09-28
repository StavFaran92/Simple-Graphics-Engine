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

	std::unordered_map<std::string, std::string> refSlots;
	if (Engine::get()->getSubSystem<ScriptSystem>()->getScriptRefSlots(*this, refSlots))
	{
		std::unordered_map<std::string, Entity> newRefs;
		for (const auto& refSlot : refSlots)
		{
			auto it = refs.find(refSlot.first);
			newRefs[refSlot.first] = (it != refs.end()) ? it->second : Entity::EmptyEntity;
		}
		refs = std::move(newRefs);
	}

}

bool ScriptComponent::isValid() const
{
	return !script.isEmpty();
}

LuaScriptAssetRef& ScriptComponent::getScript()
{
	return script;
}

void ScriptComponent::setRef(const std::string& fieldName, Entity ref)
{
	auto it = refs.find(fieldName);
	if (it != refs.end())
	{
		it->second = ref;
		return;
	}

	logWarning("Could not find field name: {}", fieldName);
}

Entity ScriptComponent::getRef(const std::string& fieldName)
{
	auto it = refs.find(fieldName);
	if (it != refs.end())
	{
		return it->second;
	}
	return Entity::EmptyEntity;
}

void ScriptComponent::init(SceneResourceRef& scene)
{
	entity.setRegistry(&scene->getRegistry());

	for (auto& [refName, e] : refs)
	{
		if (e != Entity::EmptyEntity)
		{
			e.setRegistry(&scene->getRegistry());
		}
	}
}

void ScriptComponent::postLoad(SceneResourceRef& scene)
{
	// Only refreshes the ref slots to reflect the script's structure - live script instances are
	// registered separately on simulation start / prefab instantiation.
	loadScript();
}

std::vector<std::string> ScriptComponent::getAllRefSlots() const
{
	std::vector<std::string> result;

	for (const auto& ref : refs)
	{
		result.push_back(ref.first);
	}

	return result;
}
