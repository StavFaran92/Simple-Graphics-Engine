#pragma once

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "memory/Resource.h"
#include "geometry/MeshCollection.h"

class MeshExporter
{
public:
	static std::string exportMesh(const std::string& name, Resource<MeshCollection> mesh, const aiScene* scene);
	static std::string exportMaterial(std::vector<Resource<Material>> materials, const aiScene* scene);
};