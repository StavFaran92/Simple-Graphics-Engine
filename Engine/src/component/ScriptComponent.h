#pragma once

#include "component/Component.h"
#include "core/Core.h"
#include "component/ComponentSerializer.h"
#include "scripts/LuaScript.h"

#include <string>

class EngineAPI ScriptComponent : public Component
{
public:
	ScriptComponent() = default;
	ScriptComponent(const ResourceWrapper<LuaScript>& script);
	void loadScript();

	bool isValid() const;
	ResourceWrapper<LuaScript>& getScript();

	//std::string filepath;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(script);
	}

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene&)
	{
		attachSimple<ScriptComponent>(c, entityHandler);
	}

	ResourceWrapper<LuaScript> script;


private:
};

REGISTER_COMPONENT(ScriptComponent)