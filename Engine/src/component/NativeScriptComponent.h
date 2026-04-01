#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "component/ScriptableEntity.h"

struct EngineAPI NativeScriptComponent : public Component
{
	NativeScriptComponent() = default;
	std::shared_ptr<ScriptableEntity> script = nullptr;
	Entity entity = Entity::EmptyEntity;

	template<typename T, typename... Args>
	T* bind(Args&&... args)
	{
		static_assert(std::is_base_of<ScriptableEntity, T>::value, "T must inherit from ScriptableEntity");

		script = std::make_shared< T>(std::forward<Args>(args)...);
		script->entity = entity;
		//script->onCreate();

		return static_cast<T*>(script.get());
	}

	void unbind()
	{
		if (script)
		{
			//delete script;
			script = nullptr;
		}
	}

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(entity);
		SERIALIZED_MEMBER(script);
	}

	void resolve(ResourceWrapper<Scene>& scene) override;
};

REGISTER_COMPONENT(NativeScriptComponent)