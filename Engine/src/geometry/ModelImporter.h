#pragma once

#include <unordered_map>
#include <map>
#include <memory>
#include <string>

#include "core/Core.h"
#include "geometry/Mesh.h"
#include "texture/Texture.h"
#include "runtime/Entity.h"
#include "render/Material.h"
#include "memory/Asset.h"

// Forward declerations
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
class Engine;
class TextureHandler;
class MeshCollection;
class Scene;
template<typename T>class Resource;

namespace Assimp
{
	class Importer;
}

struct ModelImportSettings : public BaseAssetParameters
{
	
};

class EngineAPI ModelImporter
{
public:
	struct ModelImportSession
	{
		std::string filepath;
		std::string fileDir;
		std::string name;
		int nodeIndex = 0;
		int childIndex = 0;
		Entity root;
		std::unordered_map<std::string, unsigned int> boneNameToIDMap;
		unsigned int boneCount = 0;
		Resource<MeshCollection> mesh;
	};

	

	struct ModelInfo
	{
		Resource<MeshCollection> mesh;
		std::map<int, Resource<Material>> materials;
		std::vector<Resource<Texture>> textures;
	};

	/** Constructor */
	ModelImporter();

	/**
	 * Import a model from a file.
	 *
	 * \param path			path to the given file
	 * \param flipTexture	should flip loaded texture
	 * \return A poitner to the newly created model
	 */
	//ModelImporter::ModelInfo import(const std::string& path, ModelImportSettings settings = {});

	/**
	 * Import a model from a file.
	 *
	 * \param path			path to the given file
	 * \param flipTexture	should flip loaded texture
	 * \return A poitner to the newly created model
	 */
	void loadModelFromFile(AssetInfo& aInfo, ModelImporter::ModelInfo& modelInfo);

	bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo);

private:
	friend class Engine;

	void loadModelFromAssimpScene(const aiScene* scene, AssetInfo& aInfo, ModelImporter::ModelInfo& modelInfo);
	void processNode(aiNode* node, const aiScene* scene, ModelImportSession& session);
	void processMesh(aiMesh* mesh, const aiScene* scene, ModelImportSession& session);
	Resource<Texture> copyAiMaterialTexture(aiMaterial* mat, aiTextureType type, const std::string& dir, std::unordered_map<std::string, Resource<Texture>>& cachedTextures, AssetInfo& aInfo);
	static Texture::TextureType getTextureType(aiTextureType type);
private:
	//std::unordered_map<std::string, std::weak_ptr<Texture>> m_texturesCache;
	std::map<uint32_t, ModelImportSession> m_sessions;
	std::shared_ptr<Assimp::Importer> m_importer = nullptr;

	std::string m_lastLoadedSceneName;
};
