#pragma once

#include "geometry/Mesh.h"

class EngineAPI SkinnedMesh : public Mesh
{
public:
	SkinnedMesh();
	~SkinnedMesh() = default;

	// Inherited via Mesh
	std::vector<glm::vec3> getPositions() const override;
	std::vector<glm::vec3> getNormals() const override;

private:
	std::vector<SkinnedVertex> vertices;
};
