#include "core/CacheSystem.h"

#include <algorithm>
#include "runtime/Context.h"
#include "core/Engine.h"

std::string normalizePath(const std::string& input)
{
    // Lexical only: keys stay project-relative, independent of the working directory
    fs::path p = fs::path(input).lexically_normal();
    std::string s = p.generic_string();

#ifdef _WIN32
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return (char)std::tolower(c); });
#endif

    return s;
}

CacheSystem::CacheSystem(std::shared_ptr<ProjectAssetRegistry> par)
{
	for (const auto& [path, uuid] : par->getPaths())
	{
		// Drop absolute keys an older version saved; each asset also has its relative key
		if (!fs::path(path).is_absolute())
			m_pathToUUID[path] = uuid;
	}
	m_nameToUUID = par->getNames();
}

void CacheSystem::clear()
{
	m_pathToUUID.clear();
	m_nameToUUID.clear();
}

UUID CacheSystem::getUUIDFromName(const std::string& name) const
{
    auto it = m_nameToUUID.find(name);
    if (it != m_nameToUUID.end())
        return it->second;

    return EMPTY_UUID;
}

void CacheSystem::addNameReference(const std::string& name, UUID uid)
{
    m_nameToUUID[name] = uid;
    Engine::get()->getContext()->getProjectAssetRegistry()->syncNames(m_nameToUUID);
}

void CacheSystem::removeNameReference(const std::string& name)
{
    m_nameToUUID.erase(name);
    Engine::get()->getContext()->getProjectAssetRegistry()->syncNames(m_nameToUUID);
}

UUID CacheSystem::getUUIDFromPath(const std::string& path) const
{
    std::string nPath = normalizePath(path);
    auto it = m_pathToUUID.find(nPath);
    if (it != m_pathToUUID.end())
        return it->second;

    return EMPTY_UUID;
}

void CacheSystem::addPathReference(const std::string& path, UUID uid)
{
    std::string nPath = normalizePath(path);
    m_pathToUUID[nPath] = uid;
    Engine::get()->getContext()->getProjectAssetRegistry()->syncPaths(m_pathToUUID);
}

void CacheSystem::removePathReference(const std::string& path)
{
    std::string nPath = normalizePath(path);
    m_pathToUUID.erase(nPath);
    Engine::get()->getContext()->getProjectAssetRegistry()->syncPaths(m_pathToUUID);
}