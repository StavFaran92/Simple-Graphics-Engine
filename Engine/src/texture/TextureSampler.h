#pragma once

#include "texture/Texture.h"
#include "memory/ResourceRef.h"
#include "core/Core.h"

struct SamplerState
{
	enum Color {
		None = 0,
		R = 1,
		G = 2,
		B = 3,
		A = 4,
	};

	int channelMaskR = Color::R;
	int channelMaskG = Color::G;
	int channelMaskB = Color::B;
	int channelMaskA = Color::A;

	int channelCount = 3;

	float xOffset = 0;
	float yOffset = 0;

	float xScale = 1;
	float yScale = 1;

	bool isActive = false;
};

class EngineAPI TextureSampler
{
public:
	TextureResourceRef texture;
	SamplerState state;
};

class EngineAPI TextureSamplerAsset
{
public:
	TextureSamplerAsset(int channelCount = 3);

	TextureAssetRef texture;
	SamplerState state;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(texture);
		SERIALIZED_MEMBER(isActive);
		SERIALIZED_MEMBER(state.channelMaskR);
		SERIALIZED_MEMBER(state.channelMaskG);
		SERIALIZED_MEMBER(state.channelMaskB);
		SERIALIZED_MEMBER(state.channelMaskA);
		SERIALIZED_MEMBER(state.channelCount);
		SERIALIZED_MEMBER(state.xOffset);
		SERIALIZED_MEMBER(state.yOffset);
		SERIALIZED_MEMBER(state.xScale);
		SERIALIZED_MEMBER(state.yScale);
	}
};