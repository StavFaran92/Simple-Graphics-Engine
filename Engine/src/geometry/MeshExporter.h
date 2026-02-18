#pragma once

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "geometry/MeshGroup.h"

class MeshExporter
{
public:
	static void exportMesh(const std::string& name, const std::string& targetDir, const aiScene* scene);
	static void exportMesh(const AssetHandle<MeshGroupAsset>& meshGroup);
	static void exportMesh(const MeshData& meshData, const std::string& targetDir);
};