#pragma once

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Resource.h"
#include "MeshCollection.h"

class MeshExporter
{
public:
	static std::string exportMesh(Resource<MeshCollection> mesh, const aiScene* scene);
	static std::string exportMaterial(std::vector<std::shared_ptr<Material>> materials, const aiScene* scene);
};