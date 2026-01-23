#pragma once

#include "memory/ResourceWrapper.h"
#include "component/Component.h"
#include "core/Core.h"
#include "component/ComponentSerializer.h"
#include "scripts/LuaScript.h"

#include <string>

class EngineAPI ScriptComponent : public Component
{
public:
	ScriptComponent() = default;
	ScriptComponent(const AssetHandle<LuaScript>& script);
	void loadScript();

	bool isValid() const;
	AssetHandle<LuaScript>& getScript();

	//std::string filepath;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(script);
		SERIALIZED_MEMBER(entity);
	}

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene);


	AssetHandle<LuaScript> script;
	Entity entity;


private:
};

REGISTER_COMPONENT(ScriptComponent)