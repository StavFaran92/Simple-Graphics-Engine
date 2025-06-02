#pragma once

#include "VertexLayout.h"
#include "VertexArrayObject.h"

class WireframeGrid
{
public:
	WireframeGrid()
	{
		VertexLayout layout;
		layout.attribs.push_back(LayoutAttribute::Positions);
		vao = VertexArrayObject(layout);

		const float step = 0.1f;
		const int gridSize = 10;
		const float maxCoord = (gridSize - 1) * step;

		// Horizontal lines (along X, Z stays fixed)
		for (int z = 0; z < gridSize; ++z)
		{
			float zCoord = z * step;

			Vertex v0, v1;
			v0.position = glm::vec3(0.0f, 0.0f, zCoord);
			v1.position = glm::vec3(maxCoord, 0.0f, zCoord);
			vao.addVertex(v0);
			vao.addVertex(v1);
		}

		// Vertical lines (along Z, X stays fixed)
		for (int x = 0; x < gridSize; ++x)
		{
			float xCoord = x * step;

			Vertex v0, v1;
			v0.position = glm::vec3(xCoord, 0.0f, 0.0f);
			v1.position = glm::vec3(xCoord, 0.0f, maxCoord);
			vao.addVertex(v0);
			vao.addVertex(v1);
		}

		vao.build();
	}

	VertexArrayObject vao;
};