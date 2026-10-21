#include "EntityCommands.h"

#include "Common.h"
#include "EntityState.h"

#include <algorithm>
#include <stdexcept>

namespace
{
	void applyAll(const std::vector<ComponentSnapshot>& snapshots)
	{
		for (const auto& snapshot : snapshots)
			applySnapshot(snapshot);
	}

	void collectSubtree(Entity e, std::vector<Entity>& out)
	{
		out.push_back(e);
		if (!e.HasComponent<Transformation>())
			return;

		for (auto& [_, child] : e.getComponent<Transformation>().getChildren())
			collectSubtree(child, out);
	}

	UUID activeSceneId()
	{
		return Engine::get()->getContext()->getActiveSceneAsset().getUID();
	}

	// Refuses when another scene is open (a dropped "Open scene" step), so the edit can't hit another scene's entity
	void requireScene(UUID scene)
	{
		UUID active = activeSceneId();
		if (scene == active)
			return;

		SceneAssetRef sceneAsset(scene);
		throw std::runtime_error(sceneAsset.isEmpty() ? std::string("its scene was deleted")
			: "it belongs to scene '" + sceneAsset.info().name + "', which isn't open");
	}
}

std::vector<ComponentSnapshot> parentTransformSnapshot(Entity e)
{
	if (!e.HasComponent<Transformation>())
		return {};

	Entity parent = e.getComponent<Transformation>().getParent();
	if (!parent.valid())
		return {};

	return { takeSnapshot(parent, "Transformation") };
}

bool reparentWithUndo(Entity child, Entity newParent, const std::string& label)
{
	auto& transform = child.getComponent<Transformation>();

	// A parent change rewrites the child's, the old parent's and the new parent's Transformation
	std::vector<Entity> touched = { child };
	Entity oldParent = transform.getParent();
	if (oldParent.valid())
		touched.push_back(oldParent);
	if (newParent.valid() && newParent != oldParent)
		touched.push_back(newParent);

	const std::string component = componentTypeName<Transformation>();
	std::vector<ComponentSnapshot> before, after;
	for (auto& e : touched)
		before.push_back(takeSnapshot(e, component));

	if (newParent.valid())
		transform.setParent(newParent);
	else
		transform.removeParent();

	bool changed = false;
	for (size_t i = 0; i < touched.size(); ++i)
	{
		after.push_back(takeSnapshot(touched[i], component));
		changed |= after.back().data != before[i].data;
	}

	// e.g. "Remove from parent" on an entity without one
	if (changed)
		CommandStack::Instance().record(std::make_unique<EditComponentsCmd>(std::move(before), std::move(after), label));
	return changed;
}

void checkHierarchyLinks(Entity e)
{
	const std::string hint = "; use set_parent to change the hierarchy";
	auto& transform = e.getComponent<Transformation>();

	Entity parent = transform.getParent();
	if (parent.handlerID() != entt::null)
	{
		if (parent == e)
			throw std::runtime_error("An entity can't be its own parent" + hint);
		if (!parent.valid() || !parent.HasComponent<Transformation>())
			throw std::runtime_error("Parent " + std::to_string(parent.handlerID()) + " doesn't exist" + hint);

		auto siblings = parent.getComponent<Transformation>().getChildren();
		if (siblings.find(e.handlerID()) == siblings.end())
			throw std::runtime_error("Parent " + std::to_string(parent.handlerID()) + " doesn't list it as a child" + hint);
	}

	for (auto& [id, child] : transform.getChildren())
	{
		if (!child.valid() || !child.HasComponent<Transformation>())
			throw std::runtime_error("Child " + std::to_string(id) + " doesn't exist" + hint);
		if (child.getComponent<Transformation>().getParent() != e)
			throw std::runtime_error("Child " + std::to_string(id) + " has another parent" + hint);
	}
}

// ---- EditComponentsCmd ----

EditComponentsCmd::EditComponentsCmd(std::vector<ComponentSnapshot> before, std::vector<ComponentSnapshot> after, std::string label,
	std::function<void()> validate)
	: m_before(std::move(before)), m_after(std::move(after)), m_label(std::move(label)), m_validate(std::move(validate)),
	  m_scene(activeSceneId())
{
}

bool EditComponentsCmd::execute(std::string&)
{
	requireScene(m_scene);
	try
	{
		applyAll(m_after);
		if (m_validate)
			m_validate();
	}
	catch (...)
	{
		// Don't leave some components changed and others not
		applyAll(m_before);
		throw;
	}
	updateScene();
	return true;
}

void EditComponentsCmd::undo()
{
	requireScene(m_scene);
	applyAll(m_before);
	updateScene();
}

// ---- EntitySubtreeCmd ----

EntitySubtreeCmd::EntitySubtreeCmd(std::string label)
	: m_label(std::move(label)), m_scene(activeSceneId())
{
}

void EntitySubtreeCmd::captureSubtree(Entity root)
{
	std::vector<Entity> entities;
	collectSubtree(root, entities);

	m_root = root.handlerID();
	m_ids.clear();
	m_components.clear();

	for (auto& e : entities)
	{
		m_ids.push_back(e.handlerID());
		auto snapshots = takeEntitySnapshot(e);
		m_components.insert(m_components.end(), snapshots.begin(), snapshots.end());
	}
}

void EntitySubtreeCmd::removeSubtree()
{
	Entity root = entityFromId(m_root);
	if (!root.valid())
		return;

	// The selection would otherwise point at a destroyed entity
	Entity selected = state.getSelectedEntity();
	if (selected.valid() && std::find(m_ids.begin(), m_ids.end(), selected.handlerID()) != m_ids.end())
		state.selectEntity(Entity::EmptyEntity);

	root.remove();
	updateScene();
}

void EntitySubtreeCmd::recreateSubtree()
{
	auto scene = Engine::get()->getContext()->getActiveScene();
	auto& registry = scene->getRegistry().get();

	for (entity_id id : m_ids)
	{
		if (registry.valid(static_cast<entt::entity>(id)))
			throw std::runtime_error("Can't recreate entity " + std::to_string(id) + ": the id is in use");
	}

	std::vector<Entity> entities;
	for (entity_id id : m_ids)
		entities.push_back(Entity{ registry.create(static_cast<entt::entity>(id)), &scene->getRegistry() });

	try
	{
		// All data first, then set them up together, like loading a scene or a prefab
		for (const auto& snapshot : m_components)
			Archiver::deserializeComponent(entityFromId(snapshot.entity), snapshot.component, snapshot.data);
		Archiver::initializeEntities(entities, scene);
	}
	catch (...)
	{
		// Don't leave half-built entities in the scene
		for (entity_id id : m_ids)
		{
			if (registry.valid(static_cast<entt::entity>(id)))
				registry.destroy(static_cast<entt::entity>(id));
		}
		updateScene();
		throw;
	}
	updateScene();
}

// ---- CreateEntitiesCmd ----

CreateEntitiesCmd::CreateEntitiesCmd(Entity root, std::string label, std::vector<ComponentSnapshot> parentBefore)
	: EntitySubtreeCmd(std::move(label))
{
	captureSubtree(root);
	m_parentWith = parentTransformSnapshot(root);
	m_parentWithout = std::move(parentBefore);
}

// Only reached on redo: the entity was created before this command was recorded
bool CreateEntitiesCmd::execute(std::string&)
{
	requireScene(m_scene);
	recreateSubtree();
	applyAll(m_parentWith);
	return true;
}

void CreateEntitiesCmd::undo()
{
	requireScene(m_scene);
	removeSubtree();
	applyAll(m_parentWithout);
}

// ---- DeleteEntitiesCmd ----

DeleteEntitiesCmd::DeleteEntitiesCmd(Entity root, std::string label)
	: EntitySubtreeCmd(std::move(label))
{
	m_root = root.handlerID();
}

bool DeleteEntitiesCmd::execute(std::string& error)
{
	requireScene(m_scene);
	Entity root = entityFromId(m_root);
	if (!root.valid())
	{
		error = "Entity " + std::to_string(m_root) + " doesn't exist";
		return false;
	}

	captureSubtree(root);
	m_parentWith = parentTransformSnapshot(root);

	removeSubtree();

	m_parentWithout.clear();
	if (!m_parentWith.empty())
		m_parentWithout.push_back(takeSnapshot(entityFromId(m_parentWith.front().entity), "Transformation"));

	return true;
}

void DeleteEntitiesCmd::undo()
{
	requireScene(m_scene);
	recreateSubtree();
	applyAll(m_parentWith);
}
