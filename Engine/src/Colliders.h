#pragma once

#include "Physics.h"

enum class ColliderType : int
{
	NONE,
	BOX,
	SPHERE,
	TERRAIN,
	MESH,
	CAPSULE

};

struct Collider
{
	Collider() = default;
	virtual ~Collider() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("layerMask", layerMask);
	}

	virtual ColliderType getType() const = 0;

	Physics::LayerMask layerMask = Physics::LayerMask::LAYER_0;
};

struct CollisionBox : public Collider
{
	CollisionBox() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("halfExtent", halfExtent);
	}

	virtual ColliderType getType() const override
	{
		return ColliderType::BOX;
	}

	glm::vec3 extents{};
};

struct CollisionSphere : public Collider
{
	CollisionSphere() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("radius", radius);
	}

	virtual ColliderType getType() const override
	{
		return ColliderType::SPHERE;
	}

	float radius = 0;
};

struct CollisionMesh : public Collider
{
	CollisionMesh() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("isConvex", isConvex);
	}

	virtual ColliderType getType() const override
	{
		return ColliderType::MESH;
	}

	bool isConvex = false;
	Resource<Mesh> mesh = Resource<Mesh>::empty;
};

struct CollisionTerrain : Collider
{
	CollisionTerrain() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER("layerMask", layerMask);
	}

	virtual ColliderType getType() const override
	{
		return ColliderType::TERRAIN;
	}
};