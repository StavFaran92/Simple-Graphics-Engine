#include "CommonTextures.h"

#include "Assets.h"
#include "Cubemap.h"
#include "Context.h"
#include "ProjectAssetRegistry.h"
#include "CacheSystem.h"

CommonTextures::CommonTextures()
{
}

void CommonTextures::close()
{
	m_textures.clear();
}

Resource<Texture> CommonTextures::getTexture(TextureType texType)
{
	return m_textures[texType];
}

CommonTextures* CommonTextures::create()
{
	auto instance = new CommonTextures();

	{
		static unsigned char* whiteColor = new unsigned char[3] { 255, 255, 255 }; // todo rethink this
		auto texture = Texture::createDummyTexture(whiteColor);
		instance->m_textures[TextureType::WHITE_1X1] = texture;

		AssetInfo aInfo;
		aInfo.uuid = texture.getUID();
		aInfo.aType = AssetType::TEXTURE;
		Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);

		Texture::writeTexture2D("SGE_TEXTURE_WHITE", texture);
	}

	{
		static unsigned char* blackColor = new unsigned char[3] { 0, 0, 0};
		auto texture = Texture::createDummyTexture(blackColor);
		instance->m_textures[TextureType::BLACK_1X1] = texture;

		AssetInfo aInfo;
		aInfo.uuid = texture.getUID();
		aInfo.aType = AssetType::TEXTURE;
		Engine::get()->getSubSystem<Assets>()->addAsset(aInfo);

		Texture::writeTexture2D("SGE_TEXTURE_BLACK", texture);
	}

	//Engine::get()->getSubSystem<Assets>()->importTexture2D("SGE_CUBEMAP_WHITE", [&]() {
	//	auto cubemap = Cubemap::createDefaultCubemap();
	//	instance->m_textures[TextureType::CUBEMAP_WHITE_1X1] = cubemap;
	//	return cubemap;
	//	});

	return instance;
}

CommonTextures* CommonTextures::load()
{
	auto instance = new CommonTextures();

	instance->m_textures[TextureType::WHITE_1X1] = Engine::get()->getMemoryManagementSystem()->get<Texture>("SGE_TEXTURE_WHITE");
	instance->m_textures[TextureType::BLACK_1X1] = Engine::get()->getMemoryManagementSystem()->get<Texture>("SGE_TEXTURE_BLACK");
	//instance->m_textures[TextureType::CUBEMAP_WHITE_1X1] = Engine::get()->getMemoryManagementSystem()->get<Texture>("SGE_CUBEMAP_WHITE");

	// TODO fix
	//Engine::get()->getContext()->getProjectAssetRegistry()->

	return instance;
}