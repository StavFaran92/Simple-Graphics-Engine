#include "Path.h"

#include "core/Engine.h"
#include "fileSystem/FileSystem.h"
#include "core/Logger.h"
#include "EnginePath.h"

EnginePath::EnginePath()
	: EnginePath("")
{
}

EnginePath::EnginePath(std::filesystem::path path)
	: Path(path, Engine::get()->getSubSystem<FileSystem>()->getEngineDirectory())
{
}
