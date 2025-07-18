#include "NativeScriptsLoader.h"

void NativeScriptsLoader::init()
{
    m_registerer = std::make_unique<NativeScriptRegisterer>();
}

ScriptableEntity* NativeScriptsLoader::getScript(const std::string& name) const
{
    auto iter = m_scriptsCallbacks.find(name);
    if (iter == m_scriptsCallbacks.end())
    {
        logError("Could not find script: " + name);
        return nullptr;
    }

    return iter->second();
}

void NativeScriptsLoader::registerScript(const std::string& name, ScriptCreatorCallback callback)
{
    m_scriptsCallbacks[name] = callback;
}

void NativeScriptsLoader::getAllScripts(std::vector<std::string>& scripts) const
{
    scripts.clear();
    scripts.reserve(m_scriptsCallbacks.size());

    for (auto& [script, _] : m_scriptsCallbacks)
    {
        scripts.push_back(script);
    }
}