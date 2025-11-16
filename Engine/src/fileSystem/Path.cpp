#include "Path.h"

#include "core/Engine.h"
#include "fileSystem/FileSystem.h"
#include "core/Logger.h"

namespace fs = std::filesystem;


Path::Path(fs::path path, fs::path root)
    : m_root(root)
{
    setPath(path);
}

std::filesystem::path Path::raw() const
{
    return fs::weakly_canonical(m_root / m_path);
}

void Path::setPath(std::filesystem::path path)
{
    fs::path candidate = fs::weakly_canonical(FileSystem::toAbsolute(path));
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