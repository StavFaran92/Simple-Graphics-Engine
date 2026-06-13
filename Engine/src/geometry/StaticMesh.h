#pragma once

#include "geometry/Mesh.h"

class EngineAPI StaticMesh : public Mesh
{
public:
	StaticMesh();
	~StaticMesh() = default;

	// Inherited via Mesh
	std::vector<glm::vec3> getPositions() const override;
	std::vector<glm::vec3> getNormals() const override;
};
