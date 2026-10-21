#include "SceneCommands.h"

#include "Common.h"
#include "EntityState.h"

#include <stdexcept>

namespace
{
	void activate(UUID uuid)
	{
		SceneAssetRef scene(uuid);
		if (scene.isEmpty())
			throw std::runtime_error("the scene was deleted");

		// The selection is an entity of the scene being left
		state.selectEntity(Entity::EmptyEntity);

		Engine::get()->getContext()->setActiveScene(scene.resource()->getID());
		setupScene();
		updateScene();
	}
}

OpenSceneCmd::OpenSceneCmd(UUID from, UUID to)
	: m_from(from), m_to(to), m_label("Open " + SceneAssetRef(to).info().name)
{
}

bool OpenSceneCmd::execute(std::string&)
{
	activate(m_to);
	return true;
}

void OpenSceneCmd::undo()
{
	activate(m_from);
}
