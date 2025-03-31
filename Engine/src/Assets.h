#pragma once

#include "Engine.h"
#include "Resource.h"
#include "Texture.h"
#include "Animation.h"
#include "Shader.h"
#include "ModelImporter.h"
#include "Core.h"

#include <functional>

class EngineAPI Assets
{
public:
	Assets();

	ModelImporter::ModelInfo importMesh(const std::string& path);
	Resource<MeshCollection> loadMesh(UUID uid, const std::string& path);
	std::vector<std::string> getAllMeshes() const;

	Resource<Texture> importTexture2D(const std::string& assetName, std::function<Resource<Texture>()> func);
	Resource<Texture> importTexture2D(const std::string& path, bool flip = false);
	Resource<Texture> loadTexture2D(UUID uid, const std::string& path);
	void addTexture2D(Resource<Texture> texture);
	void addTexture2D(const std::string& name, Resource<Texture> texture);
	std::vector<std::string> getAllTextures() const;

	Resource<Animation> importAnimation(const std::string& path);
	Resource<Animation> loadAnimation(UUID uid, const std::string& path);
	std::vector<std::string> getAllAnimations() const;

	Resource<Shader> importShader(const std::string& path);
	Resource<Shader> loadShader(UUID uid, const std::string& path);
	std::vector<std::string> getAllShaders() const;

	std::string getAlias(UUID uid) const;

private:
	Texture::TextureData extractTextureDataFromFile(const std::string& fileLocation);

private:
	std::unordered_map<UUID, Resource<MeshCollection>> m_meshes;
	std::unordered_map<UUID, Resource<Texture>> m_textures;
	std::unordered_map<UUID, Resource<Animation>> m_animations;
	std::unordered_map<UUID, Resource<Shader>> m_shaders;
};