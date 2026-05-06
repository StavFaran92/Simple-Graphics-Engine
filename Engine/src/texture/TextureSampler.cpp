#include "texture/TextureSampler.h"


#include "memory/BuiltInAssets.h"

TextureSamplerAsset::TextureSamplerAsset(int channelCount)
{
	texture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE);
	state.channelCount = channelCount;
}

TextureSampler TextureSamplerAsset::resolve() const
{
	TextureSampler sampler;
	sampler.state = state;
	sampler.texture = texture.resource();
	return sampler;
}
