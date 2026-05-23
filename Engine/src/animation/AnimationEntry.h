#pragma once

#include <string>

#include "memory/AssetAliases.h"
#include "serialize/CerealHelpers.h"
#include "animation/Animation.h"

struct AnimationTrigger
{
	int frameID = 0;
	std::string name;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER_OPTIONAL(name);
		SERIALIZED_MEMBER_OPTIONAL(frameID);
	}
};

struct AnimationEntry
{
	std::string name;
	float playbackSpeed = 1.f;
	AnimationAssetRef animation;
	std::vector<AnimationTrigger> triggers;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER_OPTIONAL(name);
		SERIALIZED_MEMBER_OPTIONAL(playbackSpeed);
		SERIALIZED_MEMBER_OPTIONAL(animation);
		SERIALIZED_MEMBER_OPTIONAL(triggers);
	}

};