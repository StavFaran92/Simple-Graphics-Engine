#pragma once

#include "texture/Texture.h"
#include "memory/Resource.h"
#include "core/Core.h"

class EngineAPI TextureSampler
{
public:
	TextureSampler(int channelCount = 3);

	Resource<Texture> texture;

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

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(texture); 
		SERIALIZED_MEMBER(channelMaskR); 
		SERIALIZED_MEMBER(channelMaskG); 
		SERIALIZED_MEMBER(channelMaskB); 
		SERIALIZED_MEMBER(channelMaskA);
		SERIALIZED_MEMBER(channelCount); 
		SERIALIZED_MEMBER(xOffset);
		SERIALIZED_MEMBER(yOffset); 
		SERIALIZED_MEMBER(xScale); 
		SERIALIZED_MEMBER(yScale);
	}
};