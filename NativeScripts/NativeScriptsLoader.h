#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "sge.h"
#include "NativeScriptsRegister.h"

class NativeScriptsLoader
{
public:
	using ScriptCreatorCallback = std::function<ScriptableEntity* ()>;

	void init();
	ScriptableEntity* getScript(const std::string& name) const;
	void registerScript(const std::string& name, ScriptCreatorCallback callback);
	void getAllScripts(std::vector<std::string>& scripts) const;

	inline static std::unique_ptr<NativeScriptsLoader> instance = std::make_unique<NativeScriptsLoader>();

//private:
	NativeScriptsLoader() = default;

private:
	std::unordered_map<std::string, ScriptCreatorCallback> m_scriptsCallbacks;

	std::unique_ptr<NativeScriptRegisterer> m_registerer;
};