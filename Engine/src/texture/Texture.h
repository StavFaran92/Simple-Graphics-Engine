#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "core/Core.h"
#include "core/ApplicationConstants.h"
#include "core/Configurations.h"
#include "memory/Resource.h"
#include "memory/Asset.h"

struct AssetInfo;

//class TextureAssetRegister
//{
//public:
//	TextureAssetRegister();
//};

class EngineAPI Texture : public Asset
{
public:
	enum class TextureType
	{
		Diffuse,
		Specular,
		Normal,

		Albedo,
		Roughness,
		Metallic,
		AmbientOcclusion,

		None
	};

	enum InternalFormat : int
	{
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

		RGB2 = 0x1907 // TODO fix
	};

	enum Format : int {
		RED = 0x1903,
		GREEN = 0x1904,
		BLUE = 0x1905,
		ALPHA = 0x1906,
		RGB = 0x1907,
		RGBA = 0x1908


	};

	enum Type : int {
		BYTE = 0x1400,
		UNSIGNED_BYTE = 0x1401,
		SHORT = 0x1402,
		UNSIGNED_SHORT = 0x1403,
		INT = 0x1404,
		UNSIGNED_INT = 0x1405,
		FLOAT = 0x1406


	};

	enum TextureKeyParams : int {
		TEXTURE_WRAP_S = 0x2802,
		TEXTURE_WRAP_T = 0x2803,
		TEXTURE_WRAP_R = 0x8072,
		TEXTURE_MAG_FILTER = 0x2800,
		TEXTURE_MIN_FILTER = 0x2801,
	};

	struct TextureData
	{
		int width = 0;
		int height = 0;
		int bpp = 0;
		InternalFormat internalFormat;
		Format format;
		int target = 0;
		Type type;
		std::map<int, int> params;
		bool genMipMap = false;
		void* data = nullptr;
		bool isHDR = false;
		bool flip = false;

		void* facesData[6]{ nullptr }; //only apply to Cubemap

		bool isTransient = false;
		std::string textureName;
	};

	struct TextureImportSettings : public BaseAssetParameters
	{
		//std::string name;
		bool genMipMap = false;
		bool flip = false;
		bool saveOnDisk = true;
		//bool isTransient = false;
		std::map<int, int> params;
	};

	struct TextureAssetAttributes
	{
		bool genMipMap = false;
		bool flip = false;
		bool isHDR = false;
		std::map<int, int> params;

		// Serialize all members into map<string, string>
		std::map<std::string, std::string> toMap() const
		{
			std::map<std::string, std::string> out;
			out["gen_mip_map"] = genMipMap ? "true" : "false";
			out["flip"] = flip ? "true" : "false";
			out["is_hdr"] = isHDR ? "true" : "false";

			for (const auto& [key, value] : params)
			{
				out["param_" + std::to_string(key)] = std::to_string(value);
			}

			return out;
		}

		TextureAssetAttributes()
		{
		}

		// Deserialize from map<string, string>
		TextureAssetAttributes(const std::map<std::string, std::string>& in)
		{
			genMipMap = in.at("gen_mip_map") == "true";
			flip = in.at("flip") == "true";
			isHDR = in.at("is_hdr") == "true";

			params.clear();
			for (const auto& [key, value] : in)
			{
				if (key.rfind("param_", 0) == 0)  // key starts with "param_"
				{
					int paramKey = std::stoi(key.substr(6));
					int paramValue = std::stoi(value);
					params[paramKey] = paramValue;
				}
			}
		}
	};

	/** Constructor */
	Texture();

	/** Copy constructor */
	Texture(const Texture& other);

	/**
	 * Create an empty texture.
	 * 
	 * \param width		The generated texture width 
	 * \param height	The generated texture height
	 * \return			A pointer to the generated texture
	 */
	static Resource<Texture> createEmptyTexture(int width, int height);

	/**
	 * Create an empty texture.
	 *
	 * \param width		The generated texture width
	 * \param height	The generated texture height
	 * \return			A pointer to the generated texture
	 */
	static Resource<Texture> createEmptyTexture(int width, int height, int internalFormat, int format, int type);

	/**
	 * Load a texture from a given file path.
	 * 
	 * \param fileLocation	The filepath location
	 * \param isFlipped		Should flip the texture vertically
	 * \return				A pointer to the loaded texture
	 */
	static Resource<Texture> create2DTextureFromFile(const std::string& fileLocation, bool flip = FLIP_TEXTURE);

	/**
	 * Create a texture using predefined data
	 *
	 * \return				A pointer to the loaded texture
	 */
	static Resource<Texture> create2DTextureFromBuffer(const TextureData& textureData);

	static Resource<Texture> create2DTextureFromBuffer(int width, int height, int internalFormat, int format, int type, std::map<int, int> params, void* data);

	static Resource<Texture> createDummyTexture(unsigned char color[3]);

	//static Texture::TextureData extractTextureDataFromFile(const std::string& fileLocation);

	void build(const TextureData& textureData);

	int getWidth() const;
	int getHeight() const;
	int getBitDepth() const;

	void setData(int xoffset, int yoffset, int width, int height, const void* data);

	/**
	 * Converts a texture type to a string.
	 * 
	 * \param type	The texture type to convert
	 * \return		A string representation of the texture type.
	 */
	static std::string textureTypeToString(TextureType type);

	/** Binds the GPU to this texture */
	void bind() const;

	/** Unbinds this texture from the GPU */
	void unbind() const;

	unsigned int getID() const;

	inline void setSlot(int slot) { m_slot = slot; }

	const TextureData& getData() const
	{
		return m_data;
	}

	TextureAssetAttributes getTextureAssetAttributes();

	//static void extractTextureDataFromFile(const std::string& fileLocation, Texture::TextureData& textureData);
	static void extractTextureDataFromSettings(const TextureImportSettings& settings, Texture::TextureData& textureData);
	//static void extractTextureDataFromAttributes(const TextureAssetAttributes& attributes, Texture::TextureData& textureData);

	static void writeTexture2D(const std::string& fileLocation, Resource<Texture> texture);
	static void addTexture2D(Resource<Texture> texture);
	static void addTexture2D(const std::string& name, Resource<Texture> texture);

	static Resource<Texture> importTexture3D(const std::string& fileLocation);

	static Resource<Texture> import(const std::string& fileLocation, const TextureImportSettings & = {});
	static Resource<Texture> loadTransient(const std::string& fileLocation, const TextureImportSettings& settings = {});

	/**  Destructor */
	~Texture();
private:
	void ClearTexture();
	friend class Cubemap;

private:
	uint32_t m_id;
	int m_slot = 0;
	
	TextureData m_data;

	TextureAssetAttributes m_attributes;

	// Inherited via Asset
	
	//Resource<Asset> load(AssetInfo aInfo) override;
};