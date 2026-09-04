#include "FileSystem.h"
#include "core/Engine.h"

FileSystem::FileSystem()
{
}

fs::path FileSystem::toAbsolute(fs::path path)
{
    return Engine::get()->getProjectDirectory() / path;
}

fs::path FileSystem::getContentDirectory() const
{
    return "Content";
}

fs::path FileSystem::getEngineDirectory() const
{
    return "Engine";
}
