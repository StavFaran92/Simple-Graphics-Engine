#pragma once

#include "component/Component.h"
#include "core/Core.h"

#include <string>

class ScriptSystem;

class EngineAPI ScriptComponent : public Component
{
public:
	void loadScript(const std::string& path);

	bool isValid() const;

	std::string filepath;


private:
	friend class ScriptSystem;
	int handlerID = -1;
};