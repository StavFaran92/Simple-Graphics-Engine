#include "ScopedPath.h"

#include "core/Engine.h"
#include "fileSystem/FileSystem.h"
#include "core/Logger.h"

namespace fs = std::filesystem;

ScopedPath::ScopedPath()
{
}

ScopedPath ScopedPath::ContentPath(std::filesystem::path path)
{
    ScopedPath sPath(path, Engine::get()->getSubSystem<FileSystem>()->getContentDirectory());
    sPath.m_type = Type::Content;
    return sPath;
}

ScopedPath ScopedPath::EnginePath(std::filesystem::path path)
{
    ScopedPath sPath(path, Engine::get()->getSubSystem<FileSystem>()->getEngineDirectory());
    sPath.m_type = Type::Engine;
    return sPath;
}

ScopedPath::ScopedPath(fs::path path, fs::path root)
    : m_root(root)
{
    setPath(path);
}

std::filesystem::path ScopedPath::relative() const
{
    return m_path;
}

std::filesystem::path ScopedPath::absolute() const
{
    return FileSystem::toAbsolute(scoped());
}

std::filesystem::path ScopedPath::scoped() const
{
    return fs::weakly_canonical(m_root / m_path);
}

void ScopedPath::setPath(std::filesystem::path path)
{
    fs::path candidate = fs::weakly_canonical(FileSystem::toAbsolute(m_root / path));
    fs::path root = fs::weakly_canonical(FileSystem::toAbsolute(m_root));

    // Compute relative path
    fs::path rel = fs::relative(candidate, root);

    // If relative path starts with ".." escape attempt
    if (!rel.empty() && *rel.begin() == "..")
    {
        // Attempted to leave content root
        logError("Invalid path specified: {}", path.generic_string());
        return; // reject
    }

    m_path = rel;
}

ScopedPath::Type ScopedPath::type() const
{
    return m_type;
}