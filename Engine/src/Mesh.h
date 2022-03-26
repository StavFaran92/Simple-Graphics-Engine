#pragma once

#include <memory>
#include <vector>

#include <GL\glew.h>

#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"

#include "VertexBufferObject.h"
#include "ElementBufferObject.h"
#include "VertexBufferLayout.h"
#include "VertexArrayObject.h"

#include "ApplicationConstants.h"

class Mesh
{
public:
	enum class LayoutAttributes
	{
		Positions,
		Normals,
		Texcoords
	};
	struct VerticesLayout
	{
		std::vector<std::pair<LayoutAttributes, int>> entries;
		size_t numOfVertices = 0;
	};
	// -------------------- Methods -------------------- //
	Mesh() {};
	//Mesh(std::shared_ptr<std::vector<Vertex>> vertices, std::shared_ptr<std::vector<unsigned int>> indices);
	//Mesh(float* vertices, size_t verticesSize, unsigned int* indices, size_t indicesSize);
	//Mesh(std::shared_ptr<std::vector<Vertex>> vertices);
	//Mesh(float* vertices, size_t verticesSize);

	void renderMesh(std::shared_ptr<Shader> shader, std::shared_ptr <IRenderer>renderer);

	void addTexture(std::shared_ptr<Texture> texture);
	void addTextures(std::vector<std::shared_ptr<Texture>> textures);

	inline std::vector<std::shared_ptr<Texture>> getTextures() { return m_textures; };

	void setRawVertices(float* vertices, VerticesLayout& layout);
	void setRawIndices(unsigned int* indices, size_t size);
	
	void SetTexturesInShader(std::shared_ptr<Shader>& shader);
	void setNumOfVertices(size_t size);
	void setPositions(std::shared_ptr<std::vector<glm::vec3>> positions);
	void setNormals(std::shared_ptr<std::vector<glm::vec3>> normals);
	void setTexcoords(std::shared_ptr<std::vector<glm::vec2>> texCoords);
	void setIndices(std::shared_ptr<std::vector<unsigned int>> indices);
	void build();

	~Mesh();
private:
	void calculateNormals();
	void clearMesh();
private:
	// -------------------- Attributes -------------------- //
	std::shared_ptr<std::vector<Vertex>>       m_vertices;
	/** Mesh positions */
	std::shared_ptr<std::vector<glm::vec3>>       m_positions;

	/** Mesh normals */
	std::shared_ptr<std::vector<glm::vec3>>       m_normals;

	/** Mesh texCoords */
	std::shared_ptr<std::vector<glm::vec2>>       m_texcoords;

	/** Mesh Indices */
	std::shared_ptr<std::vector<unsigned int>>    m_indices;

	/** Mesh textures */
	std::vector<std::shared_ptr<Texture>>         m_textures;

	// render data
	std::shared_ptr<ElementBufferObject>  m_ibo;
	std::shared_ptr < VertexBufferObject> m_vbo;
	std::shared_ptr < VertexArrayObject>  m_vao;

	size_t m_numOfVertices = 0;

	GLsizei m_indexCount = 0;
};
