#pragma once

#include "memory/ResourceRef.h"
#include "component/Component.h"
#include "core/Core.h"
#include "component/ComponentSerializer.h"
#include "scripts/LuaScript.h"

class EngineAPI ScriptComponent : public Component
{
public:
	ScriptComponent() = default;
	ScriptComponent(const LuaScriptAssetRef& script);

	std::string getName() override { return "ScriptComponent"; }
	void loadScript();

	bool isValid() const;
	LuaScriptAssetRef& getScript();

	//std::string filepath;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(script);
		SERIALIZED_MEMBER(entity);
	}

	void resolve(SceneResourceRef& scene) override;

	LuaScriptAssetRef script;
	Entity entity;


private:
};

REGISTER_COMPONENT(ScriptComponent)