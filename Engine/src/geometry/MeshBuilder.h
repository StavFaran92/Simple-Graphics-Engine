#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <memory>

#include "core/Core.h"
#include "core/Configurations.h"

#include "geometry/Mesh.h"

class MeshBuilder
{
public:
	MeshBuilder(MeshType meshType);
	MeshBuilder(const MeshData& meshData);

	MeshBuilder& addVertex(VertexVariant vertex);
	MeshBuilder& addVertices(const std::vector<VertexVariant>& vertex);

	MeshBuilder& addIndex(unsigned int index);
	MeshBuilder& addIndices(const std::vector<unsigned int>& indices);
	MeshBuilder& addIndices(const unsigned int* indices, size_t size);

	MeshBuilder& addRawVertices(const float* vertices, VertexLayout layout);

	MeshBuilder& setMaterialIndex(int index);

	std::shared_ptr<Mesh> build();
	
	/** Destructor */
	~MeshBuilder() = default;
private:
	friend class ModelBuilder;
	/** Constructor */
	
	//void enableAttribute(LayoutAttribute attribute);
	//void disableAttribute(LayoutAttribute attribute);

private:

	MeshData m_data;
};