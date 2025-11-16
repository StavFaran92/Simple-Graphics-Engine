#include "FileSystem.h"
#include "core/Engine.h"

FileSystem::FileSystem()
{
    Engine::get()->registerSubSystem<FileSystem>(this);
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
    return fs::path(Engine::get()->getProjectDirectory()) / "Engine";
}
