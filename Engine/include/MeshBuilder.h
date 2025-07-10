#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <memory>

#include "Core.h"
#include "Configurations.h"

#include "Mesh.h"
#include "VertexLayout.h"

class EngineAPI MeshBuilder
{
public:
	MeshBuilder();

	MeshBuilder& addPosition(const glm::vec3& position);
	MeshBuilder& addPositions(const std::vector<glm::vec3>& positions);
	MeshBuilder& addPositions(const float* positions, size_t size);

	MeshBuilder& addNormal(const glm::vec3& normal);
	MeshBuilder& addNormals(const std::vector<glm::vec3>& normals);
	MeshBuilder& addNormals(const float* normals, size_t size);

	MeshBuilder& addTexcoord(const glm::vec2& texCoord);
	MeshBuilder& addTexcoords(const std::vector<glm::vec2>& texCoords);
	MeshBuilder& addTexcoords(const float* texCoords, size_t size);

	MeshBuilder& addColor(const glm::vec3& color);
	MeshBuilder& addColors(const std::vector<glm::vec3>& colors);
	MeshBuilder& addColors(const float* colors, size_t size);

	MeshBuilder& addTangent(const glm::vec3& tangent);
	MeshBuilder& addTangents(const std::vector<glm::vec3>& tangents);
	MeshBuilder& addTangents(const float* tangents, size_t size);

	MeshBuilder& addIndex(unsigned int index);
	MeshBuilder& addIndices(const std::vector<unsigned int>& indices);
	MeshBuilder& addIndices(const unsigned int* indices, size_t size);

	MeshBuilder& addBoneIDs(const glm::ivec3& boneIDs);
	MeshBuilder& addBoneIDs(const std::vector<glm::ivec3>& boneIDs);

	MeshBuilder& addBoneWeights(const glm::vec3& boneWeight);
	MeshBuilder& addBoneWeights(const std::vector<glm::vec3>& boneWeights);

	MeshBuilder& addRawVertices(const float* vertices, VertexLayout layout);

	MeshBuilder& addBonesInfo(const std::vector<glm::mat4>& bonesOffsets, const std::unordered_map<std::string, unsigned int>& bonesNameToIDMap);

	MeshBuilder& setMaterialIndex(int index);

	MeshBuilder& merge(const MeshBuilder& other);

	void build(Mesh& mesh);

	static MeshBuilder& builder();
	
	/** Destructor */
	~MeshBuilder() = default;
private:
	friend class ModelBuilder;
	/** Constructor */
	
	void enableAttribute(LayoutAttribute attribute);
	void disableAttribute(LayoutAttribute attribute);

private:

	MeshData m_data;
};