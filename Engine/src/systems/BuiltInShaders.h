#pragma once

#include <map>
#include "memory/ResourceWrapper.h"
#include "geometry/MeshCollection.h"

class EngineAPI BuiltInShaders
{
public:
	BuiltInShaders();
	~BuiltInShaders() = default;
private:
	std::map<std::string, ResourceWrapper<Shader>> m_shaders;
};