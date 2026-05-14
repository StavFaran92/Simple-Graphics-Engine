#pragma once

#include "physics/Physics.h"

#include "cereal/types/optional.hpp"
#include <cereal/types/polymorphic.hpp>
#include <cereal/cereal.hpp>
#include "serialize/CerealHelpers.h"

#include "memory/ResourceRef.h"

class Mesh;

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
		SERIALIZED_MEMBER(layerMask);
	}

	virtual ColliderType getType() const = 0;

	Physics::LayerMask layerMask = Physics::LayerMask::LAYER_0;
};

struct CollisionBox : public Collider
{
	CollisionBox() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(extents);
	}

	virtual ColliderType getType() const override
	{
		return ColliderType::BOX;
	}

	glm::vec3 extents{.5f};
};

struct CollisionSphere : public Collider
{
	CollisionSphere() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(radius);
	}

	virtual ColliderType getType() const override
	{
		return ColliderType::SPHERE;
	}

	float radius = .5f;
};

struct CollisionCapsule : public Collider
{
	CollisionCapsule() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(radius);
		SERIALIZED_MEMBER(halfHeight);
	}

	virtual ColliderType getType() const override
	{
		return ColliderType::CAPSULE;
	}

	float radius = .5f;
	float halfHeight = .5f;

};

struct CollisionMesh : public Collider
{
	CollisionMesh() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(isConvex);
	}

	virtual ColliderType getType() const override
	{
		return ColliderType::MESH;
	}

	bool isConvex = false;
	ResourceRef<Mesh> mesh = ResourceRef<Mesh>::empty;
};

struct CollisionTerrain : Collider
{
	CollisionTerrain() = default;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(layerMask);
	}

	virtual ColliderType getType() const override
	{
		return ColliderType::TERRAIN;
	}
};

CEREAL_REGISTER_TYPE(CollisionTerrain);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Collider, CollisionTerrain)
CEREAL_REGISTER_TYPE(CollisionMesh);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Collider, CollisionMesh)
CEREAL_REGISTER_TYPE(CollisionSphere);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Collider, CollisionSphere)
CEREAL_REGISTER_TYPE(CollisionBox);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Collider, CollisionBox)
CEREAL_REGISTER_TYPE(CollisionCapsule);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Collider, CollisionCapsule)