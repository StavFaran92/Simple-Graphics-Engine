#include "texture/TextureBinaryLoader.h"

#include <fstream>

#include "core/Logger.h"
#include "texture/Texture.h"

namespace
{
	// Binary header describing texture metadata; kept private to this translation unit.
	struct TextureBinaryHeader
	{
		uint32_t nameLength = 0;

		int32_t width = 0;
		int32_t height = 0;
		int32_t channels = 0;
		int32_t depth = 0; // For 3D textures

		int32_t internalFormat = 0; // TextureInternalFormat as int
		int32_t format = 0;          // TextureFormat as int
		int32_t target = 0;          // TextureTarget as int
		int32_t type = 0;            // TextureType as int
		int32_t filter = 0;          // TextureFilter as int
		int32_t wrap = 0;            // TextureWrap as int

		uint8_t genMipMap = 0;
		uint8_t flip = 0;
		uint8_t fillEmpty = 0;

		uint64_t dataSize = 0;         // Size of pixel data in bytes
		uint64_t facesDataSize[6]{};   // Size of each cubemap face data (if applicable)
	};

	inline bool writeAll(std::ofstream& file, const void* data, std::size_t size)
	{
		if (size == 0)
			return true;
		file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
		return static_cast<bool>(file);
	}

	inline bool readAll(std::ifstream& file, void* data, std::size_t size)
	{
		if (size == 0)
			return true;
		file.read(reinterpret_cast<char*>(data), static_cast<std::streamsize>(size));
		return static_cast<bool>(file);
	}

} // anonymous namespace


bool TextureBinaryLoader::save(const TextureData& texture, const std::string& targetFile)
{
	std::ofstream file(targetFile, std::ios::binary);
	if (!file.is_open())
	{
		logError("TextureBinaryLoader::save - Failed to open file '{}' for writing", targetFile);
		return false;
	}

	TextureBinaryHeader header{};
	header.nameLength = static_cast<uint32_t>(texture.textureName.size());
	header.width = texture.width;
	header.height = texture.height;
	header.channels = texture.channels;
	header.depth = texture.depth;
	header.internalFormat = static_cast<int32_t>(texture.internalFormat);
	header.format = static_cast<int32_t>(texture.format);
	header.target = static_cast<int32_t>(texture.target);
	header.type = static_cast<int32_t>(texture.type);
	header.filter = static_cast<int32_t>(texture.filter);
	header.wrap = static_cast<int32_t>(texture.wrap);
	header.genMipMap = texture.genMipMap ? 1 : 0;
	header.flip = texture.flip ? 1 : 0;
	header.fillEmpty = texture.fillEmpty ? 1 : 0;

	// Sizes come from the managed buffers; header matches bytes written.
	if (texture.target == TextureTarget::TEXTURE_CUBE_MAP)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (!texture.facesData[i].empty())
			{
				header.facesDataSize[i] = static_cast<uint64_t>(texture.facesData[i].size());
			}
		}
	}
	else
	{
		if (!texture.data.empty())
		{
			header.dataSize = static_cast<uint64_t>(texture.data.size());
		}
	}

	// Write header
	if (!writeAll(file, &header, sizeof(header)))
	{
		logError("TextureBinaryLoader::save - Failed to write texture header to '{}'", targetFile);
		return false;
	}

	// Write name (no null terminator)
	if (header.nameLength > 0)
	{
		if (!writeAll(file, texture.textureName.data(), header.nameLength))
		{
			logError("TextureBinaryLoader::save - Failed to write texture name to '{}'", targetFile);
			return false;
		}
	}

	// Write pixel data
	if (texture.target == TextureTarget::TEXTURE_CUBE_MAP)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (header.facesDataSize[i] > 0)
			{
				if (texture.facesData[i].size() != static_cast<std::size_t>(header.facesDataSize[i]))
				{
					logError(
						"TextureBinaryLoader::save - Cubemap face {} size mismatch for '{}'", i, targetFile);
					return false;
				}
				if (!writeAll(file, texture.facesData[i].data(), static_cast<std::size_t>(header.facesDataSize[i])))
				{
					logError("TextureBinaryLoader::save - Failed to write cubemap face {} to '{}'", i, targetFile);
					return false;
				}
			}
		}
	}
	else
	{
		if (header.dataSize > 0)
		{
			if (texture.data.size() != static_cast<std::size_t>(header.dataSize))
			{
				logError("TextureBinaryLoader::save - Texture data size mismatch for '{}'", targetFile);
				return false;
			}
			if (!writeAll(file, texture.data.data(), static_cast<std::size_t>(header.dataSize)))
			{
				logError("TextureBinaryLoader::save - Failed to write texture data to '{}'", targetFile);
				return false;
			}
		}
	}

	return true;
}


bool TextureBinaryLoader::load(const std::string& sourceFile, TextureData& outTexture)
{
	std::ifstream file(sourceFile, std::ios::binary);
	if (!file.is_open())
	{
		logError("TextureBinaryLoader::load - Failed to open file '{}' for reading", sourceFile);
		return false;
	}

	TextureBinaryHeader header{};
	if (!readAll(file, &header, sizeof(header)))
	{
		logError("TextureBinaryLoader::load - Failed to read texture header from '{}'", sourceFile);
		return false;
	}

	// Read name
	outTexture.textureName.clear();
	if (header.nameLength > 0)
	{
		outTexture.textureName.resize(header.nameLength);
		if (!readAll(file, outTexture.textureName.data(), header.nameLength))
		{
			logError("TextureBinaryLoader::load - Failed to read texture name from '{}'", sourceFile);
			return false;
		}
	}

	// Set metadata
	outTexture.width = header.width;
	outTexture.height = header.height;
	outTexture.channels = header.channels;
	outTexture.depth = header.depth;
	outTexture.internalFormat = static_cast<TextureInternalFormat>(header.internalFormat);
	outTexture.format = static_cast<TextureFormat>(header.format);
	outTexture.target = static_cast<TextureTarget>(header.target);
	outTexture.type = static_cast<TextureType>(header.type);
	outTexture.filter = static_cast<TextureFilter>(header.filter);
	outTexture.wrap = static_cast<TextureWrap>(header.wrap);
	outTexture.genMipMap = (header.genMipMap != 0);
	outTexture.flip = (header.flip != 0);
	outTexture.fillEmpty = (header.fillEmpty != 0);

	outTexture.data.clear();
	for (int i = 0; i < 6; ++i)
	{
		outTexture.facesData[i].clear();
	}

	if (outTexture.target == TextureTarget::TEXTURE_CUBE_MAP)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (header.facesDataSize[i] > 0)
			{
				std::size_t n = static_cast<std::size_t>(header.facesDataSize[i]);
				outTexture.facesData[i].resize(n);
				if (!readAll(file, (void*)outTexture.facesData[i].data(), n))
				{
					logError("TextureBinaryLoader::load - Failed to read cubemap face {} from '{}'", i, sourceFile);
					return false;
				}
			}
		}
	}
	else
	{
		if (header.dataSize > 0)
		{
			std::size_t n = static_cast<std::size_t>(header.dataSize);
			outTexture.data.resize(n);
			if (!readAll(file, (void*)outTexture.data.data(), n))
			{
				logError("TextureBinaryLoader::load - Failed to read texture data from '{}'", sourceFile);
				outTexture.data.clear();
				return false;
			}
		}
	}

	return true;
}
