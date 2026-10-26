#pragma once

#include <vector>

#include <glm/glm.hpp>

class TextMesh
{
public:
	TextMesh(const std::vector<glm::vec4>& vertices); // xy pixels from (0, 0), zw uv
	~TextMesh();

	TextMesh(const TextMesh&) = delete;
	TextMesh& operator=(const TextMesh&) = delete;

	void draw() const;

private:
	unsigned int m_vao = 0;
	unsigned int m_vbo = 0;
	int m_vertexCount = 0;
};
