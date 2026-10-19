#include "CommandStack.h"

#include "sge.h"
#include "core/Logger.h"

#include <exception>

namespace
{
	// Snapshots are small, but an unbounded history would still grow for the whole session
	constexpr size_t kMaxHistory = 200;

	// Running or paused: undo would change the game's runtime state, not the edited scene
	bool simulationActive()
	{
		return Engine::get()->getContext()->getActiveScene()->isSimulationActive();
	}
}

CommandStack& CommandStack::Instance()
{
	static CommandStack instance;
	return instance;
}

bool CommandStack::run(std::unique_ptr<Command> command, std::string& error)
{
	try
	{
		if (!command->execute(error))
			return false;
	}
	catch (const std::exception& e)
	{
		error = e.what();
		return false;
	}

	push(std::move(command));
	return true;
}

void CommandStack::record(std::unique_ptr<Command> command)
{
	push(std::move(command));
}

bool CommandStack::undo()
{
	if (m_undo.empty() || simulationActive())
		return false;

	std::unique_ptr<Command> command = std::move(m_undo.back());
	m_undo.pop_back();

	try
	{
		command->undo();
		m_redo.push_back(std::move(command));
		return true;
	}
	catch (const std::exception& e)
	{
		logError("Undo '{}' failed: {}", command->label(), e.what());
		return false;
	}
}

bool CommandStack::redo()
{
	if (m_redo.empty() || simulationActive())
		return false;

	std::unique_ptr<Command> command = std::move(m_redo.back());
	m_redo.pop_back();

	std::string error;
	try
	{
		if (command->execute(error))
		{
			m_undo.push_back(std::move(command));
			return true;
		}
	}
	catch (const std::exception& e)
	{
		error = e.what();
	}
	logError("Redo '{}' failed: {}", command->label(), error);
	return false;
}

std::string CommandStack::undoLabel() const
{
	return m_undo.empty() ? "" : m_undo.back()->label();
}

std::string CommandStack::redoLabel() const
{
	return m_redo.empty() ? "" : m_redo.back()->label();
}

void CommandStack::clear()
{
	m_undo.clear();
	m_redo.clear();
}

void CommandStack::push(std::unique_ptr<Command> command)
{
	// A new action makes the redo history unreachable
	m_redo.clear();

	m_undo.push_back(std::move(command));
	if (m_undo.size() > kMaxHistory)
		m_undo.erase(m_undo.begin());
}
