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

void MeshExporter::exportMesh(const AssetHandle<MeshGroupAsset>& meshGroup)
{
    const AssetRecord& aInfo = meshGroup.info();
    auto& projectDir = Engine::get()->getProjectDirectory();
    const std::string savedFilePath = projectDir + "/" + aInfo.relativefilePath;

    std::ofstream out(savedFilePath);
    if (!out.is_open())
    {
        logError("Failed to open file for writing: {}", savedFilePath);
        return;
    }

    int vertexOffset = 1; // OBJ indices start at 1
    const auto& meshes = meshGroup.resource()->getMeshes();

    for (size_t meshIndex = 0; meshIndex < meshes.size(); ++meshIndex)
    {
        const auto& mesh = meshes[meshIndex];
        const auto& meshData = mesh->getMeshData();
        const auto& positions = meshData.m_positions;
        const auto& texCoords = meshData.m_texCoords;
        const auto& indices = meshData.m_indices;

        out << "o Mesh_" << meshIndex << "\n";

        // Write vertex positions
        for (const auto& pos : positions)
            out << "v " << pos.x << " " << pos.y << " " << pos.z << "\n";

        // Write texture coordinates (if available)
        if (!texCoords.empty())
        {
            for (const auto& uv : texCoords)
                out << "vt " << uv.x << " " << (1.0f - uv.y) << "\n"; // flip Y to match OBJ convention
        }

        // Write normals (optional, if you want them)
        if (!meshData.m_normals.empty())
        {
            for (const auto& n : meshData.m_normals)
                out << "vn " << n.x << " " << n.y << " " << n.z << "\n";
        }

        // Write faces
        if (!indices.empty())
        {
            // Assuming triangles (3 indices per face)
            for (size_t i = 0; i + 2 < indices.size(); i += 3)
            {
                const unsigned int i0 = vertexOffset + indices[i];
                const unsigned int i1 = vertexOffset + indices[i + 1];
                const unsigned int i2 = vertexOffset + indices[i + 2];

                // Include UVs (and normals if present)
                if (!texCoords.empty() && !meshData.m_normals.empty())
                    out << "f "
                    << i0 << "/" << i0 << "/" << i0 << " "
                    << i1 << "/" << i1 << "/" << i1 << " "
                    << i2 << "/" << i2 << "/" << i2 << "\n";
                else if (!texCoords.empty())
                    out << "f "
                    << i0 << "/" << i0 << " "
                    << i1 << "/" << i1 << " "
                    << i2 << "/" << i2 << "\n";
                else
                    out << "f "
                    << i0 << " "
                    << i1 << " "
                    << i2 << "\n";
            }
        }
        else
        {
            // No indices: use sequential vertices
            for (size_t i = 0; i + 2 < positions.size(); i += 3)
            {
                const unsigned int i0 = vertexOffset + static_cast<unsigned int>(i);
                const unsigned int i1 = vertexOffset + static_cast<unsigned int>(i + 1);
                const unsigned int i2 = vertexOffset + static_cast<unsigned int>(i + 2);

                if (!texCoords.empty() && !meshData.m_normals.empty())
                    out << "f "
                    << i0 << "/" << i0 << "/" << i0 << " "
                    << i1 << "/" << i1 << "/" << i1 << " "
                    << i2 << "/" << i2 << "/" << i2 << "\n";
                else if (!texCoords.empty())
                    out << "f "
                    << i0 << "/" << i0 << " "
                    << i1 << "/" << i1 << " "
                    << i2 << "/" << i2 << "\n";
                else
                    out << "f "
                    << i0 << " "
                    << i1 << " "
                    << i2 << "\n";
            }
        }

        vertexOffset += static_cast<int>(positions.size());
    }

    out.close();
}
