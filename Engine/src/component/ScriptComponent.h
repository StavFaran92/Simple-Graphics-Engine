#pragma once

#include "component/Component.h"
#include "core/Core.h"
#include "component/ComponentSerializer.h"

#include <string>

class ScriptSystem;

class EngineAPI ScriptComponent : public Component
{
public:
	ScriptComponent() = default;
	ScriptComponent(const std::string& path);
	void loadScript();

	bool isValid() const;

	std::string filepath;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(filepath);
	}

	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene&)
	{
		attachSimple<ScriptComponent>(c, entityHandler);
	}


private:
	friend class ScriptSystem;
	int handlerID = -1;
	//Entity entity;
};

REGISTER_COMPONENT(ScriptComponent)