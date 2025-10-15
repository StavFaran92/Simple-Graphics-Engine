#include "texture/TextureSampler.h"


#include "memory/BuiltInAssets.h"

TextureSampler::TextureSampler(int channelCount)
	: texture( BuiltInAssets::get<Texture>(SGE_TEXTURE_WHITE)),
	channelCount(channelCount)
{
}
