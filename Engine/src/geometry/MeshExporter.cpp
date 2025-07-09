#include "MeshExporter.h"

#include "Engine.h"
#include "Context.h"
#include "ProjectAssetRegistry.h"

std::string MeshExporter::exportMesh(Resource<MeshCollection> mesh, const aiScene* scene)
{
	auto& projectDir = Engine::get()->getProjectDirectory();
	Assimp::Exporter exporter;
	const std::string relativeFilepath = "/" + mesh.getUID() + ".dae";
	const std::string savedFilePath = projectDir + relativeFilepath;
	exporter.Export(scene, "collada", savedFilePath);
	return relativeFilepath;
}

std::string MeshExporter::exportMaterial(std::vector<std::shared_ptr<Material>> materials, const aiScene* scene)
{
	return "";
}