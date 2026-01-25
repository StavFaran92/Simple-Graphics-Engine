#include "memory/ResourceManager.h"

#include "core/Logger.h"

std::string ResourceManager::getRootDir() const
{
	return m_rootResourceDir;
}

void ResourceManager::setRootDir(const std::string& rootDir)
{
	m_rootResourceDir = rootDir;
}

ResourceWrapper<Resource> ResourceManager::loadResource(const std::string& fileLocation, ResourceLoadDescriptor& desc)
{
	// Validate input
	if (fileLocation.empty() || !std::filesystem::exists(fileLocation))
	{
		logError("Invalid asset path specified.");
		return ResourceWrapper<Resource>::empty;
	}

	// Load
	ResourceWrapper<Resource> resource = AssetFactory::getManager(desc.aType)->load(desc);
	if (resource.isEmpty() || !resource.get())
	{
		logError("Failed to load file {}", fileLocation);
		return ResourceWrapper<Resource>::empty;
	}

	return resource;
}