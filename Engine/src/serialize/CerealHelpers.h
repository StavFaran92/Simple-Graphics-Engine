#pragma once

#include "cereal/cereal.hpp"

#define SERIALIZED_MEMBER(member) archive(CEREAL_NVP(member));

#define SERIALIZED_MEMBER_OPTIONAL(member, value)		\
	try {												\
		archive(CEREAL_NVP(member));					\
	}													\
	catch (const cereal::Exception&) {					\
		member = value;									\
	}