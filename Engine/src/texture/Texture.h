#pragma once

#include <iostream>
#include <string>

#include "core/Core.h"
#include "memory/ResourceWrapper.h"
#include "memory/Asset.h"

using json = nlohmann::json;

struct AssetInfo;

struct TextureAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo) override;
	ResourceWrapper<ResourceBase> load(AssetInfo& aInfo) override;
	void save(const AssetWrapper<ResourceBase>& mat, const AssetInfo& aInfo) override;
	std::string getRecommendedExtension(const AssetInfo& aInfo) override;
};

class EngineAPI Texture : public ResourceBase
{
public:
	enum class TextureSemantic : int 
	{
		Color,
		Normal,
		Heightmap,
		Mask,
		Data,
		Environment,
		LUT
	};

	enum class TextureFilter
	{
		Nearest,
		Linear,
	};

	enum class TextureWrap
	{
		Repeat,
		Clamp,
		Mirror
	};

	enum class InternalFormat : int
	{
		DEPTH_COMPONENT = 0x1902,
		COMPRESSED_RED = 0x8225,
		COMPRESSED_RG = 0x8226,
		RG = 0x8227,
		RG_INTEGER = 0x8228,
		R8 = 0x8229,
		RGBA8 = 0x8058,
		R16 = 0x822A,
		RG8 = 0x822B,
		RG16 = 0x822C,
		R16F = 0x822D,
		R32F = 0x822E,
		RG16F = 0x822F,
		RG32F = 0x8230,
		R8I = 0x8231,
		R8UI = 0x8232,
		R16I = 0x8233,
		R16UI = 0x8234,
		R32I = 0x8235,
		R32UI = 0x8236,
		RG8I = 0x8237,
		RG8UI = 0x8238,
		RG16I = 0x8239,
		RG16UI = 0x823A,
		RG32I = 0x823B,
		RG32UI = 0x823C,

		RGBA32F = 0x8814,
		RGB32F = 0x8815 ,
		RGBA16F = 0x881A,
		RGB16F = 0x881B,

		RGB = 0x1907,
		RGBA = 0x1908
	};

	enum class Format : int {
		DEPTH_COMPONENT = 0x1902,
		RED = 0x1903,
		GREEN = 0x1904,
		BLUE = 0x1905,
		ALPHA = 0x1906,
		RGB = 0x1907,
		RGBA = 0x1908,
		RG = 0x8227,
	};

	enum class Type : int {
		BYTE = 0x1400,
		UNSIGNED_BYTE = 0x1401,
		SHORT = 0x1402,
		UNSIGNED_SHORT = 0x1403,
		INT = 0x1404,
		UNSIGNED_INT = 0x1405,
		FLOAT = 0x1406
	};

	enum class TextureTarget : int
	{
		TEXTURE_2D = 0x0DE1,
		TEXTURE_3D = 0x806F,
		TEXTURE_CUBE_MAP = 0x8513
	};

	struct TextureData
	{
		std::string textureName;
		int width = 0;
		int height = 0;
		int channels = 0;
		InternalFormat internalFormat = Texture::InternalFormat::RGB;
		Format format = Texture::Format::RGB;
		TextureTarget target = Texture::TextureTarget::TEXTURE_2D;
		Type type = Texture::Type::UNSIGNED_BYTE;
		TextureFilter filter = TextureFilter::Linear;
		TextureWrap wrap = TextureWrap::Repeat;
		bool genMipMap = false;
		bool flip = false;
		bool fillEmpty = false;

		int depth = 0; // Only apply to texture3D

		void* data = nullptr;
		void* facesData[6]{ nullptr }; //only apply to Cubemap
	};

	struct TextureAssetDescriptor : public AssetCreateDescriptor
	{
		bool genMipMap = false;
		bool flip = false;
		bool saveOnDisk = true;
		TextureSemantic usage = TextureSemantic::Color;
		TextureFilter filter = TextureFilter::Linear;
		TextureWrap wrap = TextureWrap::Repeat;

		json fillParams() const override
		{
			return *this;
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(TextureAssetDescriptor,
			genMipMap,
			flip,
			saveOnDisk,
			usage,
			filter,
			wrap
		);
	};

	Texture();

	static ResourceWrapper<Texture> createTexture(TextureData& textureData);

	static ResourceWrapper<Texture> createTexture(int width, 
													int height, 
													int channels, 
													InternalFormat internalFormat, 
													Format format, 
													Type type, 
													TextureFilter filter = TextureFilter::Linear, 
													TextureWrap wrap = TextureWrap::Clamp, 
													void* data = nullptr);

	static ResourceWrapper<Texture> createTexture(int width, int height, Texture::TextureSemantic usage, void* data = nullptr);

	static AssetWrapper<Texture> import(const std::string& fileLocation, TextureAssetDescriptor = {});

	static ResourceWrapper<Texture> load(const std::string& fileLocation, TextureAssetDescriptor = {});

	ResourceWrapper<Texture> clone() const; 

	int getWidth() const;
	int getHeight() const;
	int getChannels() const;

	void setData(int xoffset, int yoffset, int width, int height, const void* data);

	void generateMipMaps();

	void bind() const;
	void unbind() const;

	uint32_t getID() const;

	inline void setSlot(int slot) { m_slot = slot; }

	const TextureData& getData() const { return m_data; }

	TextureData& getData() { return m_data; }

	static Texture::Format getFormatFromChannels(int channels);

	static InternalFormat getInternalFormatFromUsage(Texture::TextureSemantic usage);

	~Texture();
private:
	friend class TextureAssetManager;

	void build(const TextureData& textureData);

	void ClearTexture();

	void setTextureParameters(const TextureData& tData);

	static void fillTextureBufferIfNeeded(TextureData& tData);
	
	static void extractTextureDataFromFile(const std::string& fileLocation, Texture::TextureData& textureData);

	static void extractTextureDataFromSettings(const TextureAssetDescriptor& settings, Texture::TextureData& textureData);

	

private:
	uint32_t m_id = 0;
	int m_slot = 0;
	
	TextureData m_data;
};

template<typename T>
GLenum toGL(T arg) {
	return static_cast<GLenum>(arg);
}