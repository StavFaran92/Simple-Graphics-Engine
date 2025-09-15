#include "geometry/MeshExporter.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "serialize/ProjectAssetRegistry.h"

void MeshExporter::exportMesh(const std::string& name, const std::string& targetDir, const aiScene* scene)
{
	if (!scene)
	{
		logError("Cannot export empty scene.");
		return;
	}
	auto& projectDir = Engine::get()->getProjectDirectory();
	Assimp::Exporter exporter;
	const std::string filename = name + ".dae";
	const std::string savedFilePath = projectDir + "/" + targetDir + "/" + filename;
	exporter.Export(scene, "collada", savedFilePath);
}