#include "Common.h"

std::unordered_map<std::string, ResourceWrapper<Texture>> icons;

std::vector<SceneObject> sceneObjects;

void suggestUniqueName(const std::string& hint, char* buffer, size_t bufferSize)
{
	std::string suggestedName = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName(hint);
	suggestedName.copy(buffer, bufferSize - 1);
	buffer[suggestedName.size() < bufferSize ? suggestedName.size() : bufferSize - 1] = '\0';
}
