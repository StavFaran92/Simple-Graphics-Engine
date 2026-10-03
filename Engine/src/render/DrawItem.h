#pragma once

#include <cstdint>

#include "glm/glm.hpp"
#include "geometry/AABB.h"

class Mesh;
class Material;

enum DrawItemFlags : uint32_t
{
	DRAW_ITEM_NONE			= 0,
	DRAW_ITEM_FORWARD		= 1 << 0,
	DRAW_ITEM_DEFERRED		= 1 << 1,
	DRAW_ITEM_TRANSPARENT	= 1 << 2,
	DRAW_ITEM_ANIMATED		= 1 << 3
};

struct DrawItem
{
	Mesh*			mesh = nullptr;
	Material*		material = nullptr;
	glm::mat4		transform{ 1.0f };	// used when not instanced
	//Buffer* instanceBuffer;  // per-instance transforms, or nullptr
	//uint32    instanceCount;   // 1 for regular objects
	//Buffer* boneBuffer;      // for skinned meshes, or nullptr
	AABB			worldBounds;		// for culling
	uint32_t		flags = DRAW_ITEM_NONE;
	uint32_t		entityId = 0;		// source entity, for picking/debug while migrating
};
