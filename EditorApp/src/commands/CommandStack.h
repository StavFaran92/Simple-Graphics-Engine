#pragma once

#include "Command.h"

#include <memory>
#include <string>
#include <vector>

// Undo/redo history of editor actions
class CommandStack
{
public:
	static CommandStack& Instance();

	// Executes the command and adds it to the history; false + error if it failed
	bool run(std::unique_ptr<Command> command, std::string& error);

	// Adds a command whose change was already applied (panels edit components live)
	void record(std::unique_ptr<Command> command);

	// False when there was nothing to apply, the simulation is running, or the command failed (logged)
	bool undo();
	bool redo();

	bool canUndo() const { return !m_undo.empty(); }
	bool canRedo() const { return !m_redo.empty(); }

	// Label of the action undo/redo would apply, empty if none
	std::string undoLabel() const;
	std::string redoLabel() const;

	// Drops the history; stored entity ids mean nothing after a scene or project load
	void clear();

private:
	void push(std::unique_ptr<Command> command);

	std::vector<std::unique_ptr<Command>> m_undo;
	std::vector<std::unique_ptr<Command>> m_redo;
};
