#include "core/CacheSystem.h"

CacheSystem::CacheSystem(std::shared_ptr<ProjectAssetRegistry> par)
{
	m_pathToUUID = par->getPaths();
	m_nameToUUID = par->getNames();
}

void CacheSystem::clear()
{
	m_pathToUUID.clear();
	m_nameToUUID.clear();
}
