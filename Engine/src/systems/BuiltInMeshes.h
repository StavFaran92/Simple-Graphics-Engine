#pragma once

#include <map>
#include "Resource.h"
#include "MeshCollection.h"

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

	Resource<MeshCollection> getMesh(MeshType type);
private:
	std::map<MeshType, Resource<MeshCollection>> m_meshes;
};