#include "EntityState.h"

#include "EditorState.h"

EntityStates state;

void EntityStates::selectEntity(Entity e)
{
	m_selectedEntity = e;
	Engine::get()->getSubSystem<ObjectPicker>()->setSelectedObject(m_selectedEntity.handlerID());

	if (e == Entity::EmptyEntity)
	{
		EditorState::Instance().setActiveEditorTool(EditorTool::Type::None);
		return;
	}

	std::shared_ptr<EntityState> eState = std::make_shared<EntityState>(e);
	eState->update();

	m_states[e.handlerID()] = eState;

	EditorState::Instance().setActiveEditorTool(EditorTool::Type::TransformTool);
}

EntityState& EntityStates::getCurrentEntityState()
{
	auto iter = m_states.find(m_selectedEntity.handlerID());
	if (iter == m_states.end())
	{
		auto eState = std::make_shared<EntityState>(m_selectedEntity);
		eState->update();
		m_states[m_selectedEntity.handlerID()] = eState;
		return *eState.get();
	}

	return *iter->second.get();
}