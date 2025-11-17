#include "systems/UniqueNameManager.h"

#include "core/Engine.h"
#include <filesystem>

#include "fileSystem/ScopedPath.h"
namespace fs = std::filesystem;

UniqueNameManager::UniqueNameManager()
{
	Engine::get()->registerSubSystem<UniqueNameManager>(this);
}

std::string UniqueNameManager::suggestUniqueName(const std::string& hint, const ScopedPath& folder) const
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
	while (isNameExists(currentNameCandidate, folder))
	{
		currentNameCandidate = origNameCandidate + "_" + std::to_string(counter);
		counter++;
	}

	return currentNameCandidate;
}

bool UniqueNameManager::isNameExists(const std::string& name, const ScopedPath& folder) const
{
	std::filesystem::path folderPath = folder.absolute();

	if (!fs::exists(folderPath) || !fs::is_directory(folderPath))
		return false; // folder doesn't exist -> no conflict

	for (const auto& entry : fs::directory_iterator(folderPath))
	{
		std::string filename = entry.path().stem().string(); // name without extension

		if (filename == name)
			return true;
	}

	return false;
}
