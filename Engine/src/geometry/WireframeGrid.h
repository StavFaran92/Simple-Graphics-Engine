#pragma once

#include "VertexLayout.h"
#include "render/VertexArrayObject.h"
#include "Shader.h"
#include "Resource.h"

class WireframeGrid
{
public:
	WireframeGrid()
	{
		VertexLayout layout;
		layout.attribs.push_back(LayoutAttribute::Positions);
		vao = std::make_shared<VertexArrayObject>();
		
		


		const float step = 1.0f;
		const int gridSize = 50;
		const float halfSize = (gridSize - 1) * step * 0.5f;

		std::vector<Vertex> vertices;
		vertices.reserve(gridSize * 2);

		// Horizontal lines (along X, Z stays fixed)
		for (int i = 0; i < gridSize; ++i)
		{
			float z = -halfSize + i * step;

			Vertex v0, v1;
			v0.position = glm::vec3(-halfSize, 0.0f, z);
			v1.position = glm::vec3(halfSize, 0.0f, z);
			vertices.push_back(v0);
			vertices.push_back(v1);
		}

		// Vertical lines (along Z, X stays fixed)
		for (int i = 0; i < gridSize; ++i)
		{
			float x = -halfSize + i * step;

			Vertex v0, v1;
			v0.position = glm::vec3(x, 0.0f, -halfSize);
			v1.position = glm::vec3(x, 0.0f, halfSize);
			vertices.push_back(v0);
			vertices.push_back(v1);
		}

		std::shared_ptr<VertexBufferObject> vbo = VertexBufferObject::create(vertices, layout);
		vao->attachBuffer(vbo, 0);
		vao->setVertexCount(vertices.size());
		vao->build();

		shader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/UnlitShader.glsl");
	}

	std::shared_ptr<VertexArrayObject> vao;
	Resource<Shader> shader;
};