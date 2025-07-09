#include "TextureSampler.h"
#include "CommonTextures.h"

TextureSampler::TextureSampler(int channelCount)
	: texture( Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::WHITE_1X1)), 
	channelCount(channelCount)
{
}
