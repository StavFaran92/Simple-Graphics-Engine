#include "geometry/MeshBinaryLoader.h"

#include <fstream>

#include "core/Logger.h"
#include "geometry/Model.h"

namespace
{
	struct ModelBinaryHeader
	{
		uint32_t meshCount = 0;
		uint32_t bonesOffsetsCount = 0;
		uint32_t bonesNameToIDCount = 0;
		uint32_t materialSlotsCount = 0;
	};

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


bool ModelBinaryLoader::save(const ModelData& modelData, const std::string& targetFile)
{
	std::ofstream file(targetFile, std::ios::binary);
	if (!file.is_open())
	{
		logError("ModelBinaryLoader::save - Failed to open file '{}' for writing", targetFile);
		return false;
	}

	ModelBinaryHeader modelHeader{};
	modelHeader.meshCount = static_cast<uint32_t>(modelData.m_meshes.size());
	modelHeader.bonesOffsetsCount = static_cast<uint32_t>(modelData.m_bonesOffsets.size());
	modelHeader.bonesNameToIDCount = static_cast<uint32_t>(modelData.m_bonesNameToIDMap.size());
	modelHeader.materialSlotsCount = static_cast<uint32_t>(modelData.m_materialSlots.size());

	if (!writeAll(file, &modelHeader, sizeof(modelHeader)))
	{
		logError("ModelBinaryLoader::save - Failed to write model header to '{}'", targetFile);
		return false;
	}

	for (const MeshData& mesh : modelData.m_meshes)
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
			logError("ModelBinaryLoader::save - Failed to write mesh header to '{}'", targetFile);
			return false;
		}

		// Write name (no null terminator)
		if (header.nameLength > 0)
		{
			if (!writeAll(file, mesh.name.data(), header.nameLength))
			{
				logError("ModelBinaryLoader::save - Failed to write mesh name to '{}'", targetFile);
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

	if (!writeAll(file, modelData.m_bonesOffsets.data(),
		modelData.m_bonesOffsets.size() * sizeof(glm::mat4))) return false;

	for (const auto& [boneName, boneID] : modelData.m_bonesNameToIDMap)
	{
		const uint32_t nameLength = static_cast<uint32_t>(boneName.size());
		if (!writeAll(file, &nameLength, sizeof(nameLength))) return false;
		if (!writeAll(file, boneName.data(), nameLength)) return false;
		if (!writeAll(file, &boneID, sizeof(boneID))) return false;
	}

	for (int slot : modelData.m_materialSlots)
	{
		if (!writeAll(file, &slot, sizeof(slot))) return false;
	}

	return true;
}


bool ModelBinaryLoader::load(const std::string& sourceFile, ModelData& outModelData)
{
	std::ifstream file(sourceFile, std::ios::binary);
	if (!file.is_open())
	{
		logError("ModelBinaryLoader::load - Failed to open file '{}' for reading", sourceFile);
		outModelData = {};
		return false;
	}

	ModelBinaryHeader modelHeader{};
	if (!readAll(file, &modelHeader, sizeof(modelHeader)))
	{
		logError("ModelBinaryLoader::load - Failed to read model header from '{}'", sourceFile);
		outModelData = {};
		return false;
	}

	outModelData = {};
	outModelData.m_meshes.resize(modelHeader.meshCount);

	for (uint32_t i = 0; i < modelHeader.meshCount; ++i)
	{
		MeshBinaryMeshHeader header{};
		if (!readAll(file, &header, sizeof(header)))
		{
			logError("ModelBinaryLoader::load - Failed to read mesh header from '{}'", sourceFile);
			outModelData = {};
			return false;
		}

		MeshData& mesh = outModelData.m_meshes[i];

		// Read name
		mesh.name.clear();
		if (header.nameLength > 0)
		{
			mesh.name.resize(header.nameLength);
			if (!readAll(file, mesh.name.data(), header.nameLength))
			{
				logError("ModelBinaryLoader::load - Failed to read mesh name from '{}'", sourceFile);
				outModelData = {};
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

	outModelData.m_bonesOffsets.resize(modelHeader.bonesOffsetsCount);
	if (!readAll(file, outModelData.m_bonesOffsets.data(),
		outModelData.m_bonesOffsets.size() * sizeof(glm::mat4)))
	{
		outModelData = {};
		return false;
	}

	for (uint32_t i = 0; i < modelHeader.bonesNameToIDCount; ++i)
	{
		uint32_t nameLength = 0;
		if (!readAll(file, &nameLength, sizeof(nameLength))) { outModelData = {}; return false; }

		std::string boneName;
		boneName.resize(nameLength);
		if (!readAll(file, boneName.data(), nameLength)) { outModelData = {}; return false; }

		unsigned int boneID = 0;
		if (!readAll(file, &boneID, sizeof(boneID))) { outModelData = {}; return false; }

		outModelData.m_bonesNameToIDMap[boneName] = boneID;
	}

	for (uint32_t i = 0; i < modelHeader.materialSlotsCount; ++i)
	{
		int slot = 0;
		if (!readAll(file, &slot, sizeof(slot))) { outModelData = {}; return false; }
		outModelData.m_materialSlots.insert(slot);
	}

	return true;
}

