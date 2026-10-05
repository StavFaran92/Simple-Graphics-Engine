#pragma once

#include <cstdint>

#include "glm/glm.hpp"
#include "geometry/AABB.h"

class Mesh;
class Material;

// Per-instance data uploaded to the GPU alongside the instanced model-matrix buffer.
// Mirrors the SSBO struct read on the GPU side - keep std430 friendly (uint, not bool).
struct InstanceData
{
	unsigned int modelIndex = 0; // index (in mat4 units) into the animation SSBO where this model's bones start
	unsigned int isAnimated = 0;
	unsigned int boneCount = 0;
	unsigned int _pad0 = 0;      // explicit padding to 16 bytes - drivers disagree on std430 struct rounding
};
static_assert(sizeof(InstanceData) == 16, "InstanceData must match the shader's std430 layout (include/buffers.glsl)");

enum DrawItemFlags : uint32_t
{
	DRAW_ITEM_NONE			= 0,
	DRAW_ITEM_FORWARD		= 1 << 0,
	DRAW_ITEM_DEFERRED		= 1 << 1,
	DRAW_ITEM_TRANSPARENT	= 1 << 2,
	DRAW_ITEM_ANIMATED		= 1 << 3,
	DRAW_ITEM_INSTANCED		= 1 << 4
};

struct DrawItem
{
	Mesh*			mesh = nullptr;
	Material*		material = nullptr;
	glm::mat4		transform{ 1.0f };	// used when not instanced
	//Buffer* instanceBuffer;  // per-instance transforms, or nullptr
	//uint32    instanceCount;   // 1 for regular objects
	uint32_t		boneOffset = 0;		// first bone matrix in the scene's draw item bones array
	uint32_t		boneCount = 0;		// 0 for non animated items
	AABB			worldBounds;		// for culling
	uint32_t		flags = DRAW_ITEM_NONE;
	uint32_t		entityId = 0;		// source entity, for picking/debug while migrating

	uint32_t		instanceOffset = 0;  // first instance in the instanced model / instance data buffers
	uint32_t		instanceCount = 1;   // 1 for regular objects
};
