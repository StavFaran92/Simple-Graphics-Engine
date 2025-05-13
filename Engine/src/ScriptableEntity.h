#pragma once

#include "Core.h"

#include "Entity.h"

#include "SDL.h"
#include "EventSystem.h"

struct NativeScriptComponent;
class Scene;
class SceneSerializer;
class EditorApp;
class Sandbox; // TODO fix this hack!

class EngineAPI ScriptableEntity
{
public:
	ScriptableEntity() = default;

	virtual ~ScriptableEntity() = default;

	std::string name() const
	{
		//todo fix (might cause issues with diff compilers)

		std::string fullName = typeid(*this).name();

		// Find the position of the space after "class"
		size_t pos = fullName.find(' ');
		if (pos != std::string::npos) {
			return fullName.substr(pos + 1);
		}

		return fullName; // Return the original string if 'class' not found
	}

	Entity entity;
	std::shared_ptr<EventSystem> m_eventSystem;

	template <class Archive>
	void serialize(Archive& archive) {
		archive(entity);
	}

protected:
	virtual void onCreate() {};
	virtual void onUpdate(float deltaTime) {};
	virtual void onDestroy() {};
	

	friend NativeScriptComponent;
	friend Scene;
	friend SceneSerializer;
	friend EditorApp;
	friend Sandbox;
};