#pragma once

#include <string>
#include <unordered_map>

#include "memory/UUID.h"

class EngineAPI ResourceManager
{
public:
	std::string getRootDir() const;
	void setRootDir(const std::string& rootDir);

    int getRefCount(UUID uuid) const;


    int incRef(UUID uuid);

    int decRef(UUID uuid);

private:
	std::string m_rootResourceDir;

    std::unordered_map<UUID, int> m_resourceRefCount;
};