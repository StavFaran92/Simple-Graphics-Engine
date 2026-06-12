#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <memory>

#include "core/Core.h"
#include "core/Configurations.h"

#include "geometry/Mesh.h"
#include "render/VertexLayout.h"

class MeshBuilder
{
public:
	MeshBuilder();
	MeshBuilder(const MeshData& meshData);

	void addVertex(StaticVertex vertex);
	void addVertex(SkinnedVertex vertex);
	void addVertices(const std::vector<StaticVertex>& vertex);
	void addVertices(const std::vector<SkinnedVertex>& vertex);

	MeshBuilder& addIndex(unsigned int index);
	MeshBuilder& addIndices(const std::vector<unsigned int>& indices);
	MeshBuilder& addIndices(const unsigned int* indices, size_t size);

	MeshBuilder& addRawVertices(const float* vertices, VertexLayout layout);

	MeshBuilder& setMaterialIndex(int index);

	std::shared_ptr<Mesh> build();

	static MeshBuilder& builder();
	
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