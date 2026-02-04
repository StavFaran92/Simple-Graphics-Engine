#pragma once

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "geometry/Model.h"

class MeshExporter
{
public:
	static void exportMesh(const std::string& name, const std::string& targetDir, const aiScene* scene);
	static void exportMesh(const AssetHandle<ModelAsset>& meshGroup);
};