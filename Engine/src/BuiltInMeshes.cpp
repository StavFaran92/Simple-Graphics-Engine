#include "BuiltInMeshes.h"

#include "Quad.h"
#include "Box.h"
#include "Sphere.h"
#include "Grid.h"

#include "Assets.h"
#include "ModelImporter.h"

BuiltInMeshes::BuiltInMeshes()
{
	{
		// Create box
		auto& mesh = Box::createMesh();
		m_meshes[MeshType::BOX] = mesh;

		//AssetInfo aInfo;
		//aInfo.uuid = mesh.getUID();
		//aInfo.aType = AssetType::MESH;
		//aInfo.name = "SGE_BOX_MESH";
		//aInfo.isTransient = true;
		//Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);
	}

	{
		// Create Quad
		auto& mesh = Quad::createMesh();
		m_meshes[MeshType::QUAD] = mesh;

		//AssetInfo aInfo;
		//aInfo.uuid = mesh.getUID();
		//aInfo.aType = AssetType::MESH;
		//aInfo.name = "SGE_QUAD_MESH";
		//aInfo.isTransient = true;
		//Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);
	}

	{
		// Create sphere
		auto& mesh = Sphere::createMesh(1, 36, 36);
		m_meshes[MeshType::SPHERE] = mesh;

		//AssetInfo aInfo;
		//aInfo.uuid = mesh.getUID();
		//aInfo.aType = AssetType::MESH;
		//aInfo.name = "SGE_SPHERE_MESH";
		//aInfo.isTransient = true;
		//Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);
	}

	{
		auto& mesh = Grid::generateGrid(10, 10, true);
		m_meshes[MeshType::GRID] = mesh;
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
