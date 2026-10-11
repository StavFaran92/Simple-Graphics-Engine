#include "texture/TextureSampler.h"


#include "memory/BuiltInAssets.h"

TextureSamplerAsset::TextureSamplerAsset(int channelCount)
{
	// Texture is left empty, resolve() falls back to white so this is safe to construct
	// before the asset database is loaded (e.g. while parsing asset records)
	state.channelCount = channelCount;
}

TextureSampler TextureSamplerAsset::resolve() const
{
	TextureSampler sampler;
	sampler.state = state;
	sampler.texture = texture.isEmpty()
		? BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource()
		: texture.resource();
	return sampler;
}
