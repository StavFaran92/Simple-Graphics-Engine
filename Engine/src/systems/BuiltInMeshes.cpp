#include "systems/BuiltInMeshes.h"

#include "Quad.h"
#include "Box.h"
#include "Sphere.h"
#include "geometry/Grid.h"

#include "Assets.h"
#include "ModelImporter.h"
#include "core/Factory.h"

void addAsAsset(const Resource<MeshCollection>& meshCollection)
{
	AssetInfo aInfo;
	aInfo.uuid = meshCollection.getUID();
	aInfo.aType = AssetType::MESH;
	aInfo.name = meshCollection.getUID();
	aInfo.isTransient = true;
	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);
}

BuiltInMeshes::BuiltInMeshes()
{
	{
		// Create box
		Resource<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_BOX");
		Box::createMesh(meshCollection);
		addAsAsset(meshCollection);
		m_meshes[MeshType::BOX] = meshCollection;
	}

	{
		// Create Quad
		Resource<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_QUAD");
		Quad::createMesh(meshCollection);
		addAsAsset(meshCollection);
		m_meshes[MeshType::QUAD] = meshCollection;
	}

	{
		// Create sphere
		Resource<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_SPHERE");
		Sphere::createMesh(meshCollection, 1, 36, 36);
		addAsAsset(meshCollection);
		m_meshes[MeshType::SPHERE] = meshCollection;
	}

	{
		Resource<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_GRID");
		Grid::generateGrid(meshCollection, 10, 10);
		addAsAsset(meshCollection);
		m_meshes[MeshType::GRID] = meshCollection;
	}
}

BuiltInMeshes::~BuiltInMeshes()
{
}

Resource<MeshCollection> BuiltInMeshes::getMesh(MeshType type)
{
	auto iter = m_meshes.find(type);
	if (iter == m_meshes.end())
	{
		logError("Invalid mesh type: " + std::to_string((int)type));
		return Resource<MeshCollection>::empty;
	}
	return (*iter).second;
}
