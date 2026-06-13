#pragma once
#include <memory>
#include <vector>

#include "core/Core.h"
#include "memory/Asset.h"
#include "render/VertexLayout.h"
#include "render/Vertex.h"
#include "geometry/AABB.h"

// Forward declerations
class MeshBuilder;
class Model;
class VertexBufferObject;
class ElementBufferObject;
class VertexArrayObject;

enum class MeshType
{
	StaticMesh = 0,
	SkinnedMesh = 1
};

using VertexVariant = std::variant<
	StaticVertex, 
	SkinnedVertex
>;

struct MeshData
{
	std::string name;
	std::vector<unsigned int> indices;
	int materialIndex{};
	glm::mat4 restTransform{ 1.0f };
	MeshType type{};
	std::vector<VertexVariant> vertices;

	size_t getStride() const
	{
		size_t stride = 0;
		for (auto entry : m_layout.attribs)
		{
			auto& attribData = getAttributeData(entry);
			stride += attribData.length * attribData.size;
		}

		return stride;
	}

private:
	friend class MeshBuilder;
	friend class ModelBinaryLoader;

	VertexLayout m_layout;

public:
	size_t getVertexCount() const
	{
		return vertices.size();
	}
	size_t getIndexCount() const
	{
		return indices.size();
	}
	bool isSkinned() const
	{
		return type == MeshType::SkinnedMesh;
	}
};

/**
 * The mesh class is used to represent the Egnine's basic mesh object,
 * it can be used to manipulate an existing Mesh data or to generate one yourself.
 * It should be used with the MeshBuilder class.
 */
class EngineAPI Mesh : public Resource
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
	virtual std::vector<glm::vec3> getPositions() const = 0;

	/**
	 * Gets the mesh's Normals vertices.
	 *
	 * \return	normals vector pointer
	 */
	virtual std::vector<glm::vec3> getNormals() const = 0;

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
