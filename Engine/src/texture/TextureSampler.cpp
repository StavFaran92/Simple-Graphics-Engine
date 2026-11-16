#include "texture/TextureSampler.h"


#include "memory/BuiltInAssets.h"
#include "render/Shader.h"

#include <unordered_map>

struct SamplerUniformNames
{
	std::string texture, isActive, xOffset, yOffset, xScale, yScale, channelMaskR, channelMaskG, channelMaskB, channelMaskA;
};

// Builds a sampler's uniform names once, shared by all samplers
static const SamplerUniformNames& getUniformNames(const std::string& name)
{
	static std::unordered_map<std::string, SamplerUniformNames> cache;

	auto it = cache.find(name);
	if (it != cache.end())
		return it->second;

	SamplerUniformNames names{
		name + ".texture",
		name + ".isActive",
		name + ".xOffset",
		name + ".yOffset",
		name + ".xScale",
		name + ".yScale",
		name + ".channelMaskR",
		name + ".channelMaskG",
		name + ".channelMaskB",
		name + ".channelMaskA"
	};

	return cache.emplace(name, std::move(names)).first->second;
}

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

void TextureSampler::apply(Shader& shader, const std::string& name, int slot) const
{
	// Local copy: the ref is const here, binding isn't
	TextureResourceRef boundTexture = texture;
	boundTexture->setSlot(slot);
	boundTexture->bind();

	const SamplerUniformNames& names = getUniformNames(name);

	shader.setUniformValue(names.texture, slot);
	shader.setUniformValue(names.isActive, state.isActive);
	shader.setUniformValue(names.xOffset, state.xOffset);
	shader.setUniformValue(names.yOffset, state.yOffset);
	shader.setUniformValue(names.xScale, state.xScale);
	shader.setUniformValue(names.yScale, state.yScale);

	// Unused channels are masked off
	shader.setUniformValue(names.channelMaskR, state.channelMaskR);
	shader.setUniformValue(names.channelMaskG, state.channelCount > 1 ? state.channelMaskG : 0);
	shader.setUniformValue(names.channelMaskB, state.channelCount > 2 ? state.channelMaskB : 0);
	shader.setUniformValue(names.channelMaskA, state.channelCount > 3 ? state.channelMaskA : 0);
}
