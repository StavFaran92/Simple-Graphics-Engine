#pragma once

#include "Physics.h"

#include "cereal/types/optional.hpp"
#include <cereal/types/polymorphic.hpp>
#include <cereal/cereal.hpp>

#define SERIALIZED_MEMBER(name, member)	archive(cereal::make_nvp(name, member));

#define SERIALIZED_MEMBER_OPTIONAL(name, member, value)	\
	try {												\
		archive(cereal::make_nvp(name, member));		\
	}													\
	catch (const cereal::Exception&) {					\
		member = value;									\
	}

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
		SERIALIZED_MEMBER("halfExtent", extents);
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
		SERIALIZED_MEMBER("radius", radius);
	}

	virtual ColliderType getType() const override
	{
		return ColliderType::SPHERE;
	}

	float radius = .5f;
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

CEREAL_REGISTER_TYPE(CollisionTerrain);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Collider, CollisionTerrain)
CEREAL_REGISTER_TYPE(CollisionMesh);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Collider, CollisionMesh)
CEREAL_REGISTER_TYPE(CollisionSphere);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Collider, CollisionSphere)
CEREAL_REGISTER_TYPE(CollisionBox);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Collider, CollisionBox)