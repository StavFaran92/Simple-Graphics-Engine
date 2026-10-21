#pragma once

#include "Command.h"
#include "memory/UUID.h"

#include <string>

// Switching the open scene; undo switches back. Scene ids are scene asset uuids.
class OpenSceneCmd : public Command
{
public:
	OpenSceneCmd(UUID from, UUID to);

	bool execute(std::string& error) override;
	void undo() override;
	std::string label() const override { return m_label; }

private:
	UUID m_from;
	UUID m_to;
	std::string m_label;
};
