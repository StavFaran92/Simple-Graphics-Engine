#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>

#include "serialize/ProjectAssetRegistry.h"

namespace fs = std::filesystem;

std::string normalizePath(const std::string& input);

class CacheSystem
{
public:
    CacheSystem(std::shared_ptr<ProjectAssetRegistry> par);
    ~CacheSystem() = default;

    UUID getUUIDFromName(const std::string& name) const;
    void addNameReference(const std::string& name, UUID uid);
    void removeNameReference(const std::string& name);

    UUID getUUIDFromPath(const std::string& path) const;
    void addPathReference(const std::string& path, UUID uid);
    void removePathReference(const std::string& path);

    void clear();

private:
    std::unordered_map<std::string, UUID> m_pathToUUID;
    std::unordered_map<std::string, UUID> m_nameToUUID;
};
