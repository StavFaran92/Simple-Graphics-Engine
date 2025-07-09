#pragma once
#include <memory>
#include <vector>

#include "Core.h"
#include "Renderer.h"
#include "Shader.h"
#include "Resource.h"

#include "VertexBufferObject.h"
#include "ElementBufferObject.h"
#include "VertexArrayObject.h"

#include "ApplicationConstants.h"

#include "Configurations.h"
#include "VertexLayout.h"
#include "AABB.h"

struct MeshData
{
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec3> m_tangents;
	std::vector<glm::vec2> m_texCoords;
	std::vector<glm::vec3> m_colors;
	std::vector<unsigned int> m_indices;
	std::vector<glm::ivec3> bonesIDs;
	std::vector<glm::vec3> bonesWeights;
	int materialIndex{};
	VertexLayout m_layout;
};

// Forward declerations
class MeshBuilder;
class Model;
struct MeshData;

/**
 * The mesh class is used to represent the Egnine's basic mesh object,
 * it can be used to manipulate an existing Mesh data or to generate one yourself.
 * It should be used with the MeshBuilder class.
 */
class EngineAPI Mesh
{
public:
	// -------------------- Methods -------------------- //
	/** Constructor */
	Mesh();

	/**
	 * Gets the number of vertices in the mesh.
	 *
	 * \param size	Number of vertices the mesh contains
	 */
	size_t getNumOfVertices() const;

	/**
	 * Gets The mesh's positions vertices.
	 *
	 * \return	positions vector pointer
	 */
	const std::vector<glm::vec3>& getPositions() const;

	/**
	 * Gets the mesh's Normals vertices.
	 *
	 * \return	normals vector pointer
	 */
	const std::vector<glm::vec3>& getNormals() const;

	/**
	 * Clear the Mesh entirely.
	 *
	 */
	void clearMesh();

	/**
	 * Sets the mesh's vertexLayout
	 *
	 */
	void setVertexLayout(VertexLayout layout);

	/**
	 * Gets the mesh's vertexLayout
	 *
	 * \return The mesh's Vertex layout
	 */
	VertexLayout getVertexLayout();

	VertexArrayObject* getVAO() const;

	AABB getAABB() const;

	int getMaterialIndex() const;

	/**
	 * Build the mesh using the specified vertices data.
	 * This call will initialize all the OpenGL related Buffer data
	 *
	 * return true if the model was built succesfully, false otherwise.
	 */
	bool build(MeshData& mData);

	/** Destructor */
	~Mesh();
private:
	// -------------------- Friends -------------------- //
	friend class Model;
	friend class MeshBuilder;

private:
	// -------------------- Methods -------------------- //
	void calculateNormals();
private:
	// -------------------- Attributes -------------------- //
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec3> m_normals;

	// render data
	std::shared_ptr<ElementBufferObject> m_ibo = nullptr;
	std::shared_ptr<VertexBufferObject>  m_vbo = nullptr;
	std::shared_ptr<VertexArrayObject>   m_vao = nullptr;

	size_t m_indexCount = 0;
	VertexLayout m_layout;
	bool m_useColors = false;

	AABB m_aabb;
	int materialIndex{};
};
