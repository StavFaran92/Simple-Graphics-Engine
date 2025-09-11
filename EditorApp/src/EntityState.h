#pragma once

#include "Common.h"

struct EntityState
{
	Entity e;

	Resource<Shader> shader;

	std::vector<std::string> animationRenameBuffers{};

	std::string renameBuffer;

	EntityState(Entity e)
		: e(e)
	{
	}

	void update()
	{
		if (e.HasComponent<Animator>())
		{
			auto& animator = e.getComponent<Animator>();
			auto animations = animator.getAllAnimations();
			animationRenameBuffers.clear();
			for (const auto& [name, anim] : animations) {
				animationRenameBuffers.push_back(name);
			}
		}

		if (e.HasComponent<ObjectComponent>())
		{
			auto& obj = e.getComponent<ObjectComponent>();
			renameBuffer = obj.name;
		}
		

	}
};

class EntityStates
{
public:
	void selectEntity(Entity e)
	{
		m_selectedEntity = e;
		Engine::get()->getSubSystem<ObjectPicker>()->setSelectedObject(m_selectedEntity.handlerID());

		if (e == Entity::EmptyEntity)
		{
			return;
		}

		std::shared_ptr<EntityState> eState = std::make_shared<EntityState>(e);
		eState->update();

		m_states[e.handlerID()] = eState;
	}

	EntityState& getCurrentEntityState()
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

	Entity getSelectedEntity() const
	{
		return m_selectedEntity;
	}

private:
	std::unordered_map<entity_id, std::shared_ptr<EntityState>> m_states;
	Entity m_selectedEntity = Entity::EmptyEntity;
};

extern EntityStates state;