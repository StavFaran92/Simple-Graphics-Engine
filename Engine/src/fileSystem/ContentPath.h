#pragma once

#include "fileSystem/Path.h"
#include "core/Core.h"

/*
A specific type of path,
is always relative to the Content root folder, guranteed to never leave it.
example values might be '/A/B', which in practice might expand to '~/user/Game/Content/A/B'
*/
class EngineAPI ContentPath : public Path
{
public:
	ContentPath();

	ContentPath(std::filesystem::path path);
};