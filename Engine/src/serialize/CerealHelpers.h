#pragma once

#include <glm/glm.hpp>
#include "cereal/cereal.hpp"
#include <cereal/types/variant.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

#define SERIALIZED_MEMBER(member) archive(CEREAL_NVP(member));

// Call this first in every derived Component::serialize to persist the base fields (e.g. isActive).
// Cereal does not chain base class serialize automatically, even with polymorphism registered.
#define SERIALIZE_COMPONENT_BASE Component::serialize(archive);

#define SERIALIZED_MEMBER_OPTIONAL(member)		\
	try {												\
		archive(CEREAL_NVP(member));					\
	}													\
	catch (const cereal::Exception&) {					\
		member = {};									\
	};

#define SERIALIZED_MEMBER_OPTIONAL2(member, value)		\
	try {												\
		archive(CEREAL_NVP(member));					\
	}													\
	catch (const cereal::Exception&) {					\
		member = value;									\
	};

namespace glm
{
	template<class Archive>
	void serialize(Archive& archive, glm::vec4& v) {
		SERIALIZED_MEMBER(v.x);
		SERIALIZED_MEMBER(v.y);
		SERIALIZED_MEMBER(v.z);
		SERIALIZED_MEMBER(v.w);
	}

	template<class Archive>
	void serialize(Archive& archive, glm::vec3& v) {
		SERIALIZED_MEMBER(v.x);
		SERIALIZED_MEMBER(v.y);
		SERIALIZED_MEMBER(v.z);
	}

	template<class Archive>
	void serialize(Archive& archive, glm::vec2& v) {
		SERIALIZED_MEMBER(v.x);
		SERIALIZED_MEMBER(v.y);
	}

	template<class Archive>
	void serialize(Archive& archive, glm::quat& q) {
		SERIALIZED_MEMBER(q.x);
		SERIALIZED_MEMBER(q.y);
		SERIALIZED_MEMBER(q.z);
		SERIALIZED_MEMBER(q.w);
	}

	template<class Archive>
	void serialize(Archive& archive, glm::mat3& m) {
		SERIALIZED_MEMBER(m[0]); // column 0 (vec3)
		SERIALIZED_MEMBER(m[1]); // column 1
		SERIALIZED_MEMBER(m[2]); // column 2
	}

	template<class Archive>
	void serialize(Archive& archive, glm::mat4& m) {
		SERIALIZED_MEMBER(m[0]); // column 0 (vec4)
		SERIALIZED_MEMBER(m[1]); // column 1
		SERIALIZED_MEMBER(m[2]); // column 2
		SERIALIZED_MEMBER(m[3]); // column 3
	}
}