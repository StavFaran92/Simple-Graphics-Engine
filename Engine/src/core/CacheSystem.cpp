#include "CacheSystem.h"

CacheSystem::CacheSystem(const std::unordered_map<std::string, UUID>& associations)
	: m_associations(associations)
{
}

void CacheSystem::clear()
{
	m_associations.clear();
}
