#pragma once

#include "Common.h"

struct EntityState
{
	Entity e;

	ResourceWrapper<Shader> shader;

	std::vector<std::string> animationRenameBuffers{};

	std::string renameBuffer;

	int terrinTempWidth = 0;
	int terrinTempHeight= 0;

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

		if (e.HasComponent<Terrain>())
		{
			auto& terrain = e.getComponent<Terrain>();
			terrinTempHeight = terrain.getHeight();
			terrinTempWidth = terrain.getWidth();
		}

	}
};

class EntityStates
{
public:
	void selectEntity(Entity e);

	EntityState& getCurrentEntityState();

	Entity getSelectedEntity() const { return m_selectedEntity; }

private:
	std::unordered_map<entity_id, std::shared_ptr<EntityState>> m_states;
	Entity m_selectedEntity = Entity::EmptyEntity;
};

extern EntityStates state;