#include "texture/TextureSampler.h"


#include "memory/BuiltInAssets.h"

TextureSamplerAsset::TextureSamplerAsset(int channelCount)
{
	texture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE);
	state.channelCount = channelCount;
}

std::shared_ptr<TextureSampler> TextureSamplerAsset::resolve() const
{
	auto sampler = std::make_shared<TextureSampler>();
	sampler->state = state;
	sampler->texture = texture.resource();
	return sampler;
}
