#pragma once

#include <string>
#include <unordered_set>

#include "core/Core.h"
#include "systems/SubSystem.h"


class EngineAPI UniqueNameManager : public SubSystem
{
public:
	UniqueNameManager();

	std::string suggestUniqueName(const std::string& hint = "") const;

	bool isNameExists(const std::string& name) const;

	void addName(const std::string& name);

	void removeName(const std::string& name);

private:
	std::unordered_set<std::string> m_uniqueNames;
};