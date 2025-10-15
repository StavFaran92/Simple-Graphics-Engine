#include "geometry/MeshExporter.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "serialize/ProjectAssetRegistry.h"

#include <fstream>
#include <filesystem>

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

void MeshExporter::exportMesh(const AssetInfo& aInfo, const ResourceWrapper<MeshCollection>& meshCollection)
{
    auto& projectDir = Engine::get()->getProjectDirectory();
    const std::string savedFilePath = projectDir + "/" + aInfo.relativefilePath;

    std::ofstream out(savedFilePath);
    if (!out.is_open()) 
    {
        logError("Failed to open file for writing: {}", savedFilePath);
        return;
    }

    int vertexOffset = 1; // OBJ indices start at 1
    const auto& meshes = meshCollection.get()->getMeshes();

    for (size_t meshIndex = 0; meshIndex < meshes.size(); ++meshIndex) {
        const auto& mesh = meshes[meshIndex];
        const auto& positions = mesh->getPositions();

        out << "o Mesh_" << meshIndex << "\n";

        // Write vertex positions
        for (const auto& pos : positions) {
            out << "v " << pos.x << " " << pos.y << " " << pos.z << "\n";
        }

        // Write faces
        if (mesh->getMeshData().m_indices.size() > 0) {
            const auto& indices = mesh->getMeshData().m_indices;

            // Assuming triangles (3 indices per face)
            for (size_t i = 0; i + 2 < indices.size(); i += 3) {
                out << "f "
                    << vertexOffset + indices[i] << " "
                    << vertexOffset + indices[i + 1] << " "
                    << vertexOffset + indices[i + 2]  << "\n";
            }
        }
        else {
            // No indices: use vertex order
            for (size_t i = 0; i + 2 < positions.size(); i += 3) {
                out << "f "
                    << vertexOffset + i << " "
                    << vertexOffset + i + 1 << " "
                    << vertexOffset + i + 2 << "\n";
            }
        }

        vertexOffset += static_cast<int>(positions.size());
    }

    out.close();
}