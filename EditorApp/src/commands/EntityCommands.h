#pragma once

#include "Command.h"
#include "ComponentSnapshot.h"

#include <string>
#include <vector>

// Component changes on existing entities: undo restores "before", redo restores "after"
class EditComponentsCmd : public Command
{
public:
	EditComponentsCmd(std::vector<ComponentSnapshot> before, std::vector<ComponentSnapshot> after, std::string label);

	bool execute(std::string& error) override;
	void undo() override;
	std::string label() const override { return m_label; }

private:
	std::vector<ComponentSnapshot> m_before;
	std::vector<ComponentSnapshot> m_after;
	std::string m_label;
};

// An entity and its children that can be removed and recreated with the same ids
class EntitySubtreeCmd : public Command
{
public:
	std::string label() const override { return m_label; }

protected:
	explicit EntitySubtreeCmd(std::string label) : m_label(std::move(label)) {}

	// Snapshots every component of the root and its descendants
	void captureSubtree(Entity root);

	void removeSubtree();
	void recreateSubtree();

	entity_id m_root = 0;
	std::vector<entity_id> m_ids;
	std::vector<ComponentSnapshot> m_components;

	// The parent's Transformation with and without the subtree as a child (empty when there is no parent)
	std::vector<ComponentSnapshot> m_parentWith;
	std::vector<ComponentSnapshot> m_parentWithout;

	std::string m_label;
};

// Records an entity that was just created by existing code; undo removes it and its children
class CreateEntitiesCmd : public EntitySubtreeCmd
{
public:
	// parentBefore: the parent's Transformation snapshot taken before creating, when the entity has a parent
	CreateEntitiesCmd(Entity root, std::string label, std::vector<ComponentSnapshot> parentBefore = {});

	bool execute(std::string& error) override;
	void undo() override;
};

// Deletes an entity and its children; undo recreates them with the same ids
class DeleteEntitiesCmd : public EntitySubtreeCmd
{
public:
	DeleteEntitiesCmd(Entity root, std::string label);

	bool execute(std::string& error) override;
	void undo() override;
};

// Snapshot of the entity's parent's Transformation, or empty when it has no parent
std::vector<ComponentSnapshot> parentTransformSnapshot(Entity e);

// Sets the parent (an invalid newParent detaches) and records it as one undo step; false if nothing changed
bool reparentWithUndo(Entity child, Entity newParent, const std::string& label);
