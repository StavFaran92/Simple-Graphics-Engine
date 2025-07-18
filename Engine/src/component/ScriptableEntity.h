#pragma once

#include "core/Core.h"

#include "runtime/Entity.h"

#include "SDL.h"
#include "core/Subscriber.h"

struct NativeScriptComponent;
class Scene;
class SceneSerializer;
class EditorApp;
class Sandbox; // TODO fix this hack!

class EngineAPI ScriptableEntity : public Subscriber
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
	uint64_t eventHandler;

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