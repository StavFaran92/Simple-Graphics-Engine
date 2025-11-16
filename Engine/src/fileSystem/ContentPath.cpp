#include "ContentPath.h"

#include "fileSystem/FileSystem.h"
#include "core/Engine.h"

ContentPath::ContentPath()
	: ContentPath("")
{ }

ContentPath::ContentPath(fs::path path)
	: Path(path, Engine::get()->getSubSystem<FileSystem>()->getContentDirectory())
{
}
