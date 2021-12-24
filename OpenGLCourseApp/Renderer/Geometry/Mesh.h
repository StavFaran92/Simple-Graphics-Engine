#pragma once

#include <memory>

#include <GL\glew.h>

#include "Renderer/Renderer.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/Texture.h"

#include "Renderer/Buffers/VertexBufferObjectWrapper.h"
#include "Renderer/Buffers/ElementBufferObjectWrapper.h"
#include "Renderer/Buffers/VertexBufferLayout.h"
#include "Renderer/Buffers/VertexArrayObjectWrapper.h"



class Mesh
{  
public:
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	
	void RenderMesh(Shader& shader, const Renderer& renderer);

	~Mesh();
private:
	void ClearMesh();
private:
	// mesh data
	std::vector<Vertex>       m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Texture>      m_textures;

	// render data
	std::shared_ptr<ElementBufferObjectWrapper>  m_ibo;
	std::shared_ptr < VertexBufferObjectWrapper> m_vbo;
	std::shared_ptr < VertexArrayObjectWrapper>  m_vao;

	GLsizei m_indexCount = 0;
};

