#pragma once

#include <string>

#include "Context.h"

class ProjectManager
{
public:
	void loadProject(const std::string& filePath, std::shared_ptr<Context>& context);
	void saveProject();
};