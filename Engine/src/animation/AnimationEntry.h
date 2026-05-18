#pragma once

#include <string>

#include "memory/AssetAliases.h"
#include "serialize/CerealHelpers.h"
#include "animation/Animation.h"

struct AnimationEntry
{
	std::string name;
	float playbackSpeed = 1.f;
	AnimationAssetRef animation;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(name);
		SERIALIZED_MEMBER(playbackSpeed);
		SERIALIZED_MEMBER(animation);
	}

};