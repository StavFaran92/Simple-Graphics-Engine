#pragma once

#include <map>
#include "memory/ResourceWrapper.h"
#include "geometry/MeshCollection.h"

class EngineAPI BuiltInMeshes
{
public:
	enum class MeshType
	{
		BOX,
		SPHERE,
		QUAD,
		CAMERA,
		GRID
	};

	BuiltInMeshes();
	~BuiltInMeshes();

	ResourceWrapper<MeshCollection> getMesh(MeshType type);
private:
	std::map<MeshType, ResourceWrapper<MeshCollection>> m_meshes;
};