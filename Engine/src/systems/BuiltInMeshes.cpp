#include "systems/BuiltInMeshes.h"

#include "geometry/Quad.h"
#include "geometry/Box.h"
#include "geometry/Sphere.h"
#include "geometry/Grid.h"

#include "memory/Assets.h"
#include "geometry/ModelImporter.h"
#include "core/Factory.h"

void addAsAsset(const ResourceWrapper<MeshCollection>& meshCollection)
{
	AssetDescriptor aInfo;
	aInfo.customUUID = meshCollection.getUID();
	aInfo.aType = AssetType::MESH;
	aInfo.name = meshCollection.getUID();
	aInfo.isTransient = true;
	Engine::get()->getSubSystem<Assets>()->addAsset(aInfo.parse());
}

BuiltInMeshes::BuiltInMeshes()
{
	{
		// Create box
		ResourceWrapper<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_BOX");
		Box::createMesh(meshCollection);
		addAsAsset(meshCollection);
		m_meshes[MeshType::BOX] = meshCollection;
	}

	{
		// Create Quad
		ResourceWrapper<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_QUAD");
		Quad::createMesh(meshCollection);
		addAsAsset(meshCollection);
		m_meshes[MeshType::QUAD] = meshCollection;
	}

	{
		// Create sphere
		ResourceWrapper<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_SPHERE");
		Sphere::createMesh(meshCollection, 1, 36, 36);
		addAsAsset(meshCollection);
		m_meshes[MeshType::SPHERE] = meshCollection;
	}

	{
		ResourceWrapper<MeshCollection> meshCollection = Factory<MeshCollection>::createUsingCustomUUID("SGE_MESH_GRID");
		Grid::generateGrid(meshCollection, 10, 10);
		addAsAsset(meshCollection);
		m_meshes[MeshType::GRID] = meshCollection;
	}

	{
		ModelImportSettings settings;
		settings.customUUID = "SGE_MESH_CAMERA";
		settings.isTransient = true;
		ResourceWrapper<MeshCollection> meshCollection = MeshCollection::import(SGE_ROOT_DIR + "Resources/Engine/Meshes/camera_v2.dae", settings);
		addAsAsset(meshCollection);
		m_meshes[MeshType::CAMERA] = meshCollection;
	}
}

BuiltInMeshes::~BuiltInMeshes()
{
}

ResourceWrapper<MeshCollection> BuiltInMeshes::getMesh(MeshType type)
{
	auto iter = m_meshes.find(type);
	if (iter == m_meshes.end())
	{
		logError("Invalid mesh type: " + std::to_string((int)type));
		return ResourceWrapper<MeshCollection>::empty;
	}
	return (*iter).second;
}
