#include "systems/UniqueNameManager.h"

#include "core/Engine.h"

UniqueNameManager::UniqueNameManager()
{
	Engine::get()->registerSubSystem<UniqueNameManager>(this);
}

std::string UniqueNameManager::suggestUniqueName(const std::string& hint) const
{
	std::string origNameCandidate;
	std::string currentNameCandidate;
	if (hint.empty())
	{
		origNameCandidate = "Temp";
	}
	else
	{
		origNameCandidate = hint;
	}

	currentNameCandidate = origNameCandidate;

	int counter = 0;
	while (isNameExists(currentNameCandidate))
	{
		currentNameCandidate = origNameCandidate + "_" + std::to_string(counter);
		counter++;
	}

	return currentNameCandidate;
}

bool UniqueNameManager::isNameExists(const std::string& name) const
{
	return m_uniqueNames.find(name) != m_uniqueNames.end();
}

void UniqueNameManager::addName(const std::string& name)
{
	m_uniqueNames.insert(name);
}

void UniqueNameManager::removeName(const std::string& name)
{
	m_uniqueNames.erase(name);
}
