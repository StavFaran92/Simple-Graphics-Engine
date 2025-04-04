#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "Core.h"
#include "ApplicationConstants.h"
#include "Configurations.h"
#include "Resource.h"



class EngineAPI Texture
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

		void* facesData[6]{ nullptr }; //only apply to Cubemap
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

	static Texture::TextureData extractTextureDataFromFile(const std::string& fileLocation);

	static void writeTexture2D(const std::string& fileLocation, Resource<Texture> texture);
	static Resource<Texture> importTexture2D(const std::string& fileLocation);
	static Resource<Texture> loadTexture2D(UUID uid, const std::string& path);
	static void addTexture2D(Resource<Texture> texture);
	static void addTexture2D(const std::string& name, Resource<Texture> texture);

	/**  Destructor */
	~Texture();
private:
	void ClearTexture();
	friend class Cubemap;

private:
	uint32_t m_id;
	int m_slot = 0;
	
	TextureData m_data;
};
