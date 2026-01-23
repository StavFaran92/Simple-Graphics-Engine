#pragma once
#include <memory>
#include <vector>

#include "core/Core.h"
#include "memory/Asset.h"
#include "render/VertexLayout.h"
#include "geometry/AABB.h"

// Forward declerations
class MeshBuilder;
class Model;
class VertexBufferObject;
class ElementBufferObject;
class VertexArrayObject;

struct MeshData
{
	std::vector<glm::vec3> m_positions;
	std::vector<glm::vec3> m_normals;
	std::vector<glm::vec4> m_tangents;
	std::vector<glm::vec2> m_texCoords;
	std::vector<glm::vec3> m_colors;
	std::vector<unsigned int> m_indices;
	std::vector<glm::ivec3> bonesIDs;
	std::vector<glm::vec3> bonesWeights;
	int materialIndex{};
	VertexLayout m_layout;
};

/**
 * The mesh class is used to represent the Egnine's basic mesh object,
 * it can be used to manipulate an existing Mesh data or to generate one yourself.
 * It should be used with the MeshBuilder class.
 */
class EngineAPI Mesh : public ResourceBase
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

	const MeshData& getMeshData() const
	{
		return m_meshData;
	}

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

	void setRestTransform(glm::mat4 transform);

	glm::mat4 getRestTransform() const;

	void setName(const std::string& name);
	std::string getName() const;

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
	MeshData m_meshData;

	// render data
	std::shared_ptr<ElementBufferObject> m_ibo = nullptr;
	std::shared_ptr<VertexBufferObject>  m_vbo = nullptr;
	std::shared_ptr<VertexArrayObject>   m_vao = nullptr;

	size_t m_indexCount = 0;
	VertexLayout m_layout;
	bool m_useColors = false;
	glm::mat4 m_restTransform{ 1.0f };

	AABB m_aabb;
	int materialIndex{};
	std::string m_name;
};
