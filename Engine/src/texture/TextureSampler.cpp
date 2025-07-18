#include "texture/TextureSampler.h"
#include "systems/CommonTextures.h"

TextureSampler::TextureSampler(int channelCount)
	: texture( Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::WHITE_1X1)), 
	channelCount(channelCount)
{
}
