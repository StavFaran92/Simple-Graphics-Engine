#pragma once

#include <vector>

#include "glm/glm.hpp"

struct StaticVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec4 tangent;
};

struct SkinnedVertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec4 tangent;
	std::vector<glm::ivec3> bonesIDs;
	std::vector<glm::vec3> bonesWeights;
};