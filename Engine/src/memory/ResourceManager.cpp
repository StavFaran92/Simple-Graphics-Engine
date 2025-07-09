#include "ResourceManager.h"

std::string ResourceManager::getRootDir() const
{
	return m_rootResourceDir;
}

void ResourceManager::setRootDir(const std::string& rootDir)
{
	m_rootResourceDir = rootDir;
}

