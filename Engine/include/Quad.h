#pragma once
#include "core/Core.h"
#include "geometry/Mesh.h"

class MeshBuilder;

class EngineAPI Quad 
{
public:
	static Resource<MeshCollection> createMesh();
};

