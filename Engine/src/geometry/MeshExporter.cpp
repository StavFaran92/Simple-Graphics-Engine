#include "geometry/MeshExporter.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "serialize/ProjectAssetRegistry.h"

std::string MeshExporter::exportMesh(const std::string& name, Resource<MeshCollection> mesh, const aiScene* scene)
{
	auto& projectDir = Engine::get()->getProjectDirectory();
	Assimp::Exporter exporter;
	const std::string relativeFilepath = name + ".dae";
	const std::string savedFilePath = projectDir + "/" + relativeFilepath;
	exporter.Export(scene, "collada", savedFilePath);
	return relativeFilepath;
}

std::string MeshExporter::exportMaterial(std::vector<Resource<Material>> materials, const aiScene* scene)
{
	return "";
}