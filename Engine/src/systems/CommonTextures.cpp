#include "systems/CommonTextures.h"

#include "memory/Assets.h"
#include "texture/Cubemap.h"
#include "runtime/Context.h"
#include "serialize/ProjectAssetRegistry.h"
#include "core/CacheSystem.h"
#include <gl/glew.h>

CommonTextures::CommonTextures()
{
}

void CommonTextures::close()
{
	m_textures.clear();
}

ResourceWrapper<Texture> CommonTextures::getTexture(TextureType texType)
{
	return m_textures[texType];
}

CommonTextures* CommonTextures::create()
{
	auto instance = new CommonTextures();

	{
		static unsigned char* whiteColor = new unsigned char[3] { 255, 255, 255 }; // todo rethink this

		Texture::TextureData tData;
		tData.target = Texture::TextureTarget::TEXTURE_2D;
		tData.width = 1;
		tData.height = 1;
		tData.bpp = 3;
		tData.data = whiteColor;
		tData.internalFormat = Texture::InternalFormat::RGB2;
		tData.format = Texture::Format::RGB;
		tData.type = Texture::Type::UNSIGNED_BYTE;
		tData.isTransient = true;
		tData.textureName = "SGE_TEXTURE_WHITE";
		tData.params = { {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
						{GL_TEXTURE_MAG_FILTER, GL_LINEAR},
						{GL_TEXTURE_WRAP_S, GL_REPEAT},
						{GL_TEXTURE_WRAP_T, GL_REPEAT } };

		auto texture = Texture::create2DTextureFromBuffer(tData);
		instance->m_textures[TextureType::WHITE_1X1] = texture;

		//auto& projectDir = Engine::get()->getProjectDirectory();
		//std::string savedFileLocation = projectDir + "/" + texture.getUID() + ".png";
		//Texture::writeTexture2D(savedFileLocation, texture);

		AssetDescriptor aInfo;
		aInfo.customUUID = texture.getUID();
		aInfo.aType = AssetType::TEXTURE;
		aInfo.name = "SGE_TEXTURE_WHITE";
		aInfo.isTransient = true;
		//aInfo.filePath = savedFileLocation;
		aInfo.attributes = texture->getTextureAssetAttributes().toMap();
		Engine::get()->getSubSystem<Assets>()->addAsset(aInfo.parse());
	}

	{
		static unsigned char* blackColor = new unsigned char[3] { 0, 0, 0};
		Texture::TextureData tData;
		tData.target = Texture::TextureTarget::TEXTURE_2D;
		tData.width = 1;
		tData.height = 1;
		tData.bpp = 3;
		tData.data = blackColor;
		tData.internalFormat = Texture::InternalFormat::RGB2;
		tData.format = Texture::Format::RGB;
		tData.type = Texture::Type::UNSIGNED_BYTE;
		tData.isTransient = true;
		tData.textureName = "SGE_TEXTURE_BLACK";
		tData.params = { {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
						{GL_TEXTURE_MAG_FILTER, GL_LINEAR},
						{GL_TEXTURE_WRAP_S, GL_REPEAT},
						{GL_TEXTURE_WRAP_T, GL_REPEAT } };
		auto texture = Texture::create2DTextureFromBuffer(tData);
		instance->m_textures[TextureType::BLACK_1X1] = texture;

		//auto& projectDir = Engine::get()->getProjectDirectory();
		//std::string savedFileLocation = projectDir + "/" + texture.getUID() + ".png";
		//Texture::writeTexture2D(savedFileLocation, texture);

		AssetDescriptor aInfo;
		aInfo.customUUID = texture.getUID();
		aInfo.aType = AssetType::TEXTURE;
		aInfo.name = "SGE_TEXTURE_BLACK";
		aInfo.isTransient = true;
		//aInfo.filePath = savedFileLocation;
		aInfo.attributes = texture->getTextureAssetAttributes().toMap();
		Engine::get()->getSubSystem<Assets>()->addAsset(aInfo.parse());
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