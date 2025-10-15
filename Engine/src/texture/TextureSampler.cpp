#include "texture/TextureSampler.h"
#include "systems/CommonTextures.h"

TextureSampler::TextureSampler(int channelCount)
	: texture( Engine::get()->getCommonTextures()->getTexture("SGE_TEXTURE_WHITE")),
	channelCount(channelCount)
{
}
