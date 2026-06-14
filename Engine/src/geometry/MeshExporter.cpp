//#include "geometry/MeshExporter.h"
//
//#include "core/Engine.h"
//#include "runtime/Context.h"
//#include "serialize/ProjectAssetRegistry.h"
//
//#include <fstream>
//#include <filesystem>
//#include "core/Logger.h"
//
//void MeshExporter::exportMesh(const std::string& name, const std::string& targetDir, const aiScene* scene)
//{
//	if (!scene)
//	{
//		logError("Cannot export empty scene.");
//		return;
//	}
//	auto& projectDir = Engine::get()->getProjectDirectory();
//	Assimp::Exporter exporter;
//	const std::string filename = name + ".dae";
//	const std::string savedFilePath = projectDir + "/" + targetDir + "/" + filename;
//	exporter.Export(scene, "collada", savedFilePath);
//}
//
//void MeshExporter::exportMesh(const ModelAssetRef& meshGroup)
//{
//    // Broken
//    // 
//    // 
//    // 
//    // 
//    // 
//    //const AssetRecord& aInfo = meshGroup.info();
//    //auto& projectDir = Engine::get()->getProjectDirectory();
//    //const std::string savedFilePath = projectDir + "/" + aInfo.relativefilePath;
//
//    //std::ofstream out(savedFilePath);
//    //if (!out.is_open())
//    //{
//    //    logError("Failed to open file for writing: {}", savedFilePath);
//    //    return;
//    //}
//
//    //int vertexOffset = 1; // OBJ indices start at 1
//    //const auto& meshes = meshGroup.resource()->getMeshes();
//
//    //for (size_t meshIndex = 0; meshIndex < meshes.size(); ++meshIndex)
//    //{
//    //    const auto& mesh = meshes[meshIndex];
//    //    const auto& meshData = mesh->getMeshData();
//    //    
//    //}
//
//    //out.close();
//}
//
//void MeshExporter::exportMeshes(
//    const std::vector<MeshData>& meshes,
//    const std::string& targetFile)
//{
//    std::ofstream out(targetFile);
//    if (!out.is_open())
//    {
//        logError("Failed to open file for writing: {}", targetFile);
//        return;
//    }
//
//    // OBJ GLOBAL OFFSETS
//    uint32_t vOffset = 1; // positions
//    uint32_t vtOffset = 1; // uvs
//    uint32_t vnOffset = 1; // normals
//
//    size_t meshIndex = 0;
//
//    for (const MeshData& meshData : meshes)
//    {
//        out << "o Mesh_" << meshIndex++ << "\n";
//
//        const auto& positions = meshData.m_positions;
//        const auto& texCoords = meshData.m_texCoords;
//        const auto& normals = meshData.m_normals;
//        const auto& indices = meshData.m_indices;
//
//        // ----------------------
//        // Write vertices
//        // ----------------------
//        for (const auto& pos : positions)
//            out << "v " << pos.x << " " << pos.y << " " << pos.z << "\n";
//
//        // ----------------------
//        // Write UVs
//        // ----------------------
//        for (const auto& uv : texCoords)
//            out << "vt " << uv.x << " " << (1.0f - uv.y) << "\n";
//
//        // ----------------------
//        // Write normals
//        // ----------------------
//        for (const auto& n : normals)
//            out << "vn " << n.x << " " << n.y << " " << n.z << "\n";
//
//        // ----------------------
//        // Write faces
//        // ----------------------
//        auto writeFace = [&](uint32_t i0, uint32_t i1, uint32_t i2)
//            {
//                uint32_t v0 = vOffset + i0;
//                uint32_t v1 = vOffset + i1;
//                uint32_t v2 = vOffset + i2;
//
//                if (!texCoords.empty() && !normals.empty())
//                {
//                    uint32_t t0 = vtOffset + i0;
//                    uint32_t t1 = vtOffset + i1;
//                    uint32_t t2 = vtOffset + i2;
//
//                    uint32_t n0 = vnOffset + i0;
//                    uint32_t n1 = vnOffset + i1;
//                    uint32_t n2 = vnOffset + i2;
//
//                    out << "f "
//                        << v0 << "/" << t0 << "/" << n0 << " "
//                        << v1 << "/" << t1 << "/" << n1 << " "
//                        << v2 << "/" << t2 << "/" << n2 << "\n";
//                }
//                else if (!texCoords.empty())
//                {
//                    uint32_t t0 = vtOffset + i0;
//                    uint32_t t1 = vtOffset + i1;
//                    uint32_t t2 = vtOffset + i2;
//
//                    out << "f "
//                        << v0 << "/" << t0 << " "
//                        << v1 << "/" << t1 << " "
//                        << v2 << "/" << t2 << "\n";
//                }
//                else
//                {
//                    out << "f "
//                        << v0 << " "
//                        << v1 << " "
//                        << v2 << "\n";
//                }
//            };
//
//        if (!indices.empty())
//        {
//            for (size_t i = 0; i + 2 < indices.size(); i += 3)
//                writeFace(indices[i], indices[i + 1], indices[i + 2]);
//        }
//        else
//        {
//            // non-indexed mesh
//            for (uint32_t i = 0; i + 2 < positions.size(); i += 3)
//                writeFace(i, i + 1, i + 2);
//        }
//
//        // ----------------------
//        // Update global offsets
//        // ----------------------
//        vOffset += static_cast<uint32_t>(positions.size());
//        vtOffset += static_cast<uint32_t>(texCoords.size());
//        vnOffset += static_cast<uint32_t>(normals.size());
//    }
//}
//
