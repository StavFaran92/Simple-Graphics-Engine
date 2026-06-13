#include "geometry/MeshBinaryLoader.h"

#include <fstream>
#include <variant>

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

		uint32_t type = 0;
		uint32_t vertexCount = 0;
		uint32_t indicesCount = 0;
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
		header.nameLength      = static_cast<uint32_t>(mesh.name.size());
		header.type            = static_cast<uint32_t>(mesh.type);
		header.vertexCount     = static_cast<uint32_t>(mesh.getVertexCount());
		header.indicesCount    = static_cast<uint32_t>(mesh.indices.size());
		header.attributesCount = static_cast<uint32_t>(mesh.m_layout.attribs.size());
		header.materialIndex   = static_cast<int32_t>(mesh.materialIndex);

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

		// Indices
		if (!writeAll(file, mesh.indices.data(), header.indicesCount * sizeof(unsigned int))) return false;

		// Vertices
		if (mesh.type == MeshType::StaticMesh)
		{
			for (const VertexVariant& v : mesh.vertices)
			{
				const StaticVertex& vertex = std::get<StaticVertex>(v);
				if (!writeAll(file, &vertex, sizeof(StaticVertex))) return false;
			}
		}
		else if (mesh.type == MeshType::SkinnedMesh)
		{
			for (const VertexVariant& v : mesh.vertices)
			{
				const SkinnedVertex& vertex = std::get<SkinnedVertex>(v);

				if (!writeAll(file, &vertex.position, sizeof(vertex.position))) return false;
				if (!writeAll(file, &vertex.normal,   sizeof(vertex.normal)))   return false;
				if (!writeAll(file, &vertex.texCoord, sizeof(vertex.texCoord))) return false;
				if (!writeAll(file, &vertex.tangent,  sizeof(vertex.tangent)))  return false;

				uint32_t boneIDsCount     = static_cast<uint32_t>(vertex.bonesIDs.size());
				uint32_t boneWeightsCount = static_cast<uint32_t>(vertex.bonesWeights.size());
				if (!writeAll(file, &boneIDsCount,     sizeof(boneIDsCount)))     return false;
				if (!writeAll(file, &boneWeightsCount, sizeof(boneWeightsCount))) return false;

				if (!writeAll(file, vertex.bonesIDs.data(),     boneIDsCount     * sizeof(glm::ivec3))) return false;
				if (!writeAll(file, vertex.bonesWeights.data(), boneWeightsCount * sizeof(glm::vec3)))  return false;
			}
		}
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

		mesh.type = static_cast<MeshType>(header.type);
		mesh.materialIndex = header.materialIndex;

		// Vertex layout
		mesh.m_layout.attribs.resize(header.attributesCount);
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

		// Indices
		mesh.indices.resize(header.indicesCount);
		if (!readAll(file, mesh.indices.data(), header.indicesCount * sizeof(unsigned int))) return false;

		// Vertices
		mesh.vertices.clear();
		mesh.vertices.reserve(header.vertexCount);

		if (mesh.type == MeshType::StaticMesh)
		{
			for (uint32_t v = 0; v < header.vertexCount; ++v)
			{
				StaticVertex vertex{};
				if (!readAll(file, &vertex, sizeof(StaticVertex)))
				{
					outModelData = {};
					return false;
				}
				mesh.vertices.emplace_back(vertex);
			}
		}
		else if (mesh.type == MeshType::SkinnedMesh)
		{
			for (uint32_t v = 0; v < header.vertexCount; ++v)
			{
				SkinnedVertex vertex{};

				if (!readAll(file, &vertex.position, sizeof(vertex.position))) { outModelData = {}; return false; }
				if (!readAll(file, &vertex.normal,   sizeof(vertex.normal)))   { outModelData = {}; return false; }
				if (!readAll(file, &vertex.texCoord, sizeof(vertex.texCoord))) { outModelData = {}; return false; }
				if (!readAll(file, &vertex.tangent,  sizeof(vertex.tangent)))  { outModelData = {}; return false; }

				uint32_t boneIDsCount = 0;
				uint32_t boneWeightsCount = 0;
				if (!readAll(file, &boneIDsCount,     sizeof(boneIDsCount)))     { outModelData = {}; return false; }
				if (!readAll(file, &boneWeightsCount, sizeof(boneWeightsCount))) { outModelData = {}; return false; }

				vertex.bonesIDs.resize(boneIDsCount);
				vertex.bonesWeights.resize(boneWeightsCount);

				if (!readAll(file, vertex.bonesIDs.data(),     boneIDsCount     * sizeof(glm::ivec3))) { outModelData = {}; return false; }
				if (!readAll(file, vertex.bonesWeights.data(), boneWeightsCount * sizeof(glm::vec3)))  { outModelData = {}; return false; }

				mesh.vertices.emplace_back(std::move(vertex));
			}
		}
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
