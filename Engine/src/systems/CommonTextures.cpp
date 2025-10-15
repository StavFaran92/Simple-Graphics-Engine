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

ResourceWrapper<Texture> CommonTextures::getTexture(const std::string& name)
{
	auto iter = m_textures.find(name);
	if (iter != m_textures.end())
	{
		return iter->second;
	}

	logWarning("Could not find built in texture {}", name);

	return {};
}

void CommonTextures::acquireTexture(const std::string& name, const std::string& path)
{
	Texture::TextureAssetDescriptor aDesc;
	aDesc.isEngineOwned = true;
	aDesc.name = name;
	aDesc.customUUID = name;
	auto& texture = Texture::import(path, aDesc);
	m_textures[name] = texture;
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
		tData.isEngineOwned = true;
		tData.textureName = "SGE_TEXTURE_WHITE";
		tData.params = { {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
						{GL_TEXTURE_MAG_FILTER, GL_LINEAR},
						{GL_TEXTURE_WRAP_S, GL_REPEAT},
						{GL_TEXTURE_WRAP_T, GL_REPEAT } };

		auto texture = Texture::create2DTextureFromBuffer(tData);
		instance->m_textures[tData.textureName] = texture;

		AssetCreateDescriptor aInfo;
		aInfo.customUUID = texture.getUID();
		aInfo.aType = AssetType::TEXTURE;
		aInfo.name = tData.textureName;
		aInfo.isEngineOwned = true;
		aInfo.attributes = texture->getTextureAssetAttributes().toMap();
		Engine::get()->getSubSystem<Assets>()->createAsset(texture, aInfo);
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
		tData.isEngineOwned = true;
		tData.textureName = "SGE_TEXTURE_BLACK";
		tData.params = { {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
						{GL_TEXTURE_MAG_FILTER, GL_LINEAR},
						{GL_TEXTURE_WRAP_S, GL_REPEAT},
						{GL_TEXTURE_WRAP_T, GL_REPEAT } };
		auto texture = Texture::create2DTextureFromBuffer(tData);
		instance->m_textures[tData.textureName] = texture;

		AssetCreateDescriptor aInfo;
		aInfo.customUUID = texture.getUID();
		aInfo.aType = AssetType::TEXTURE;
		aInfo.name = tData.textureName;
		aInfo.isEngineOwned = true;
		aInfo.attributes = texture->getTextureAssetAttributes().toMap();
		Engine::get()->getSubSystem<Assets>()->createAsset(texture, aInfo);
	}

	instance->acquireTexture("SGE_TEXTURE_GRASS", SGE_ROOT_DIR + "Resources/Engine/Textures/Ground037_1K-JPG_Color.jpg");

	return instance;
}

CommonTextures* CommonTextures::load()
{
	auto instance = new CommonTextures();

	instance->m_textures["SGE_TEXTURE_WHITE"] = Engine::get()->getMemoryManagementSystem()->get<Texture>("SGE_TEXTURE_WHITE");
	instance->m_textures["SGE_TEXTURE_BLACK"] = Engine::get()->getMemoryManagementSystem()->get<Texture>("SGE_TEXTURE_BLACK");
	//instance->m_textures[TextureType::CUBEMAP_WHITE_1X1] = Engine::get()->getMemoryManagementSystem()->get<Texture>("SGE_CUBEMAP_WHITE");

	// TODO fix
	//Engine::get()->getContext()->getProjectAssetRegistry()->

	return instance;
}