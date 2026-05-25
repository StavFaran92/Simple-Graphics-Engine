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

	void setRef(const std::string& fieldName, Entity ref);
	Entity getRef(const std::string& fieldName);
	std::vector<std::string> getAllRefSlots() const;

	//std::string filepath;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZE_COMPONENT_BASE;
		SERIALIZED_MEMBER_OPTIONAL(script);
		SERIALIZED_MEMBER_OPTIONAL(entity);
		SERIALIZED_MEMBER_OPTIONAL(refs);
	}

	void resolve(SceneResourceRef& scene) override;
	void postLoad(SceneResourceRef& scene) override;

	LuaScriptAssetRef script;
	Entity entity;


private:

	std::unordered_map<std::string, Entity> refs;
};

REGISTER_COMPONENT(ScriptComponent)