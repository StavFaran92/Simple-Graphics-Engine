#pragma once

#include <string>

// An undoable editor action
class Command
{
public:
	virtual ~Command() = default;

	// Applies the action; false + error if it couldn't
	virtual bool execute(std::string& error) = 0;

	virtual void undo() = 0;

	// Shown in the Edit menu, e.g. "PhysicsComponent edit"
	virtual std::string label() const = 0;
};
