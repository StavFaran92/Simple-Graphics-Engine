#include "geometry/MeshBinaryLoader.h"

#include <fstream>

#include "core/Logger.h"
#include "geometry/Mesh.h"

namespace
{
	// Per-mesh header describing vector sizes; kept private to this translation unit.
	struct MeshBinaryMeshHeader
	{
		uint32_t nameLength = 0;

		uint32_t positionsCount = 0;
		uint32_t normalsCount = 0;
		uint32_t tangentsCount = 0;
		uint32_t texCoordsCount = 0;
		uint32_t colorsCount = 0;
		uint32_t indicesCount = 0;
		uint32_t boneIDsCount = 0;
		uint32_t boneWeightsCount = 0;

		uint32_t attributesCount = 0;

		int32_t materialIndex = 0;
	};

	inline bool writeAll(std::ofstream& file, const void* data, std::size_t size)
	{
		if (size == 0)
			return true;
		file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
		return static_cast<bool>(file);
	}

	inline bool readAll(std::ifstream& file, void* data, std::size_t size)
	{
		if (size == 0)
			return true;
		file.read(reinterpret_cast<char*>(data), static_cast<std::streamsize>(size));
		return static_cast<bool>(file);
	}

} // anonymous namespace


bool MeshBinaryLoader::save(const std::vector<MeshData>& meshes, const std::string& targetFile)
{
	std::ofstream file(targetFile, std::ios::binary);
	if (!file.is_open())
	{
		logError("MeshBinaryLoader::save - Failed to open file '{}' for writing", targetFile);
		return false;
	}

	// Write mesh count
	uint32_t meshCount = static_cast<uint32_t>(meshes.size());
	if (!writeAll(file, &meshCount, sizeof(meshCount)))
	{
		logError("MeshBinaryLoader::save - Failed to write mesh count to '{}'", targetFile);
		return false;
	}

	for (const MeshData& mesh : meshes)
	{
		MeshBinaryMeshHeader header{};
		header.nameLength       = static_cast<uint32_t>(mesh.name.size());
		header.positionsCount   = static_cast<uint32_t>(mesh.m_positions.size());
		header.normalsCount     = static_cast<uint32_t>(mesh.m_normals.size());
		header.tangentsCount    = static_cast<uint32_t>(mesh.m_tangents.size());
		header.texCoordsCount   = static_cast<uint32_t>(mesh.m_texCoords.size());
		header.colorsCount      = static_cast<uint32_t>(mesh.m_colors.size());
		header.indicesCount     = static_cast<uint32_t>(mesh.m_indices.size());
		header.boneIDsCount     = static_cast<uint32_t>(mesh.bonesIDs.size());
		header.boneWeightsCount = static_cast<uint32_t>(mesh.bonesWeights.size());
		header.attributesCount  = static_cast<uint32_t>(mesh.m_layout.attribs.size());
		header.materialIndex    = static_cast<int32_t>(mesh.materialIndex);

		// Write header
		if (!writeAll(file, &header, sizeof(header)))
		{
			logError("MeshBinaryLoader::save - Failed to write mesh header to '{}'", targetFile);
			return false;
		}

		// Write name (no null terminator)
		if (header.nameLength > 0)
		{
			if (!writeAll(file, mesh.name.data(), header.nameLength))
			{
				logError("MeshBinaryLoader::save - Failed to write mesh name to '{}'", targetFile);
				return false;
			}
		}

		// Write vertex data
		if (!writeAll(file, mesh.m_positions.data(), header.positionsCount * sizeof(glm::vec3))) return false;
		if (!writeAll(file, mesh.m_normals.data(),   header.normalsCount   * sizeof(glm::vec3))) return false;
		if (!writeAll(file, mesh.m_tangents.data(),  header.tangentsCount  * sizeof(glm::vec4))) return false;
		if (!writeAll(file, mesh.m_texCoords.data(), header.texCoordsCount * sizeof(glm::vec2))) return false;
		if (!writeAll(file, mesh.m_colors.data(),    header.colorsCount    * sizeof(glm::vec3))) return false;

		// Bones
		if (!writeAll(file, mesh.bonesIDs.data(),     header.boneIDsCount     * sizeof(glm::ivec3))) return false;
		if (!writeAll(file, mesh.bonesWeights.data(), header.boneWeightsCount * sizeof(glm::vec3)))  return false;

		// Indices
		if (!writeAll(file, mesh.m_indices.data(), header.indicesCount * sizeof(unsigned int))) return false;

		// Vertex layout
		if (!writeAll(file, mesh.m_layout.attribs.data(),
			header.attributesCount * sizeof(LayoutAttribute)))
			return false;

		// numOfVertices and stride
		uint64_t numOfVertices = static_cast<uint64_t>(mesh.m_layout.numOfVertices);
		uint64_t stride        = static_cast<uint64_t>(mesh.m_layout.stride);
		if (!writeAll(file, &numOfVertices, sizeof(numOfVertices))) return false;
		if (!writeAll(file, &stride,        sizeof(stride)))        return false;

		// Rest transform
		if (!writeAll(file, &mesh.restTransform, sizeof(mesh.restTransform))) return false;
	}

	return true;
}


bool MeshBinaryLoader::load(const std::string& sourceFile, std::vector<MeshData>& outMeshes)
{
	std::ifstream file(sourceFile, std::ios::binary);
	if (!file.is_open())
	{
		logError("MeshBinaryLoader::load - Failed to open file '{}' for reading", sourceFile);
		outMeshes.clear();
		return false;
	}

	uint32_t meshCount = 0;
	if (!readAll(file, &meshCount, sizeof(meshCount)))
	{
		logError("MeshBinaryLoader::load - Failed to read mesh count from '{}'", sourceFile);
		outMeshes.clear();
		return false;
	}

	outMeshes.clear();
	outMeshes.resize(meshCount);

	for (uint32_t i = 0; i < meshCount; ++i)
	{
		MeshBinaryMeshHeader header{};
		if (!readAll(file, &header, sizeof(header)))
		{
			logError("MeshBinaryLoader::load - Failed to read mesh header from '{}'", sourceFile);
			outMeshes.clear();
			return false;
		}

		MeshData& mesh = outMeshes[i];

		// Read name
		mesh.name.clear();
		if (header.nameLength > 0)
		{
			mesh.name.resize(header.nameLength);
			if (!readAll(file, mesh.name.data(), header.nameLength))
			{
				logError("MeshBinaryLoader::load - Failed to read mesh name from '{}'", sourceFile);
				outMeshes.clear();
				return false;
			}
		}

		// Resize containers
		mesh.m_positions.resize(header.positionsCount);
		mesh.m_normals.resize(header.normalsCount);
		mesh.m_tangents.resize(header.tangentsCount);
		mesh.m_texCoords.resize(header.texCoordsCount);
		mesh.m_colors.resize(header.colorsCount);
		mesh.m_indices.resize(header.indicesCount);
		mesh.bonesIDs.resize(header.boneIDsCount);
		mesh.bonesWeights.resize(header.boneWeightsCount);
		mesh.m_layout.attribs.resize(header.attributesCount);

		mesh.materialIndex = header.materialIndex;

		// Read vertex data
		if (!readAll(file, mesh.m_positions.data(), header.positionsCount * sizeof(glm::vec3))) return false;
		if (!readAll(file, mesh.m_normals.data(),   header.normalsCount   * sizeof(glm::vec3))) return false;
		if (!readAll(file, mesh.m_tangents.data(),  header.tangentsCount  * sizeof(glm::vec4))) return false;
		if (!readAll(file, mesh.m_texCoords.data(), header.texCoordsCount * sizeof(glm::vec2))) return false;
		if (!readAll(file, mesh.m_colors.data(),    header.colorsCount    * sizeof(glm::vec3))) return false;

		// Bones
		if (!readAll(file, mesh.bonesIDs.data(),     header.boneIDsCount     * sizeof(glm::ivec3))) return false;
		if (!readAll(file, mesh.bonesWeights.data(), header.boneWeightsCount * sizeof(glm::vec3)))  return false;

		// Indices
		if (!readAll(file, mesh.m_indices.data(), header.indicesCount * sizeof(unsigned int))) return false;

		// Vertex layout
		if (!readAll(file, mesh.m_layout.attribs.data(),
			header.attributesCount * sizeof(LayoutAttribute)))
			return false;

		// numOfVertices and stride
		uint64_t numOfVertices = 0;
		uint64_t stride        = 0;
		if (!readAll(file, &numOfVertices, sizeof(numOfVertices))) return false;
		if (!readAll(file, &stride,        sizeof(stride)))        return false;

		mesh.m_layout.numOfVertices = static_cast<std::size_t>(numOfVertices);
		mesh.m_layout.stride        = static_cast<std::size_t>(stride);

		// Rest transform
		if (!readAll(file, &mesh.restTransform, sizeof(mesh.restTransform))) return false;
	}

	return true;
}

