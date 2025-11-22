#pragma once

#include <glm/glm.hpp>
#include "cereal/cereal.hpp"

#define SERIALIZED_MEMBER(member) archive(CEREAL_NVP(member));

#define SERIALIZED_MEMBER_OPTIONAL(member, value)		\
	try {												\
		archive(CEREAL_NVP(member));					\
	}													\
	catch (const cereal::Exception&) {					\
		member = value;									\
	}

namespace glm
{
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
};

