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
template<typename T>class ResourceWrapper;

namespace Assimp
{
	class Importer;
}

struct ModelImportSettings : public AssetCreateDescriptor
{
	
};

class EngineAPI ModelImporter
{
public:
	struct ModelLoadSession
	{
		std::string filepath;
		std::string fileDir;
		std::string name;
		int nodeIndex = 0;
		int childIndex = 0;
		Entity root;
		std::unordered_map<std::string, unsigned int> boneNameToIDMap;
		unsigned int boneCount = 0;
		ResourceWrapper<MeshCollection> mesh;
	};

	struct LastImportedMaterials
	{
		std::map<int, AssetWrapper<Material>> materials;
	};

	

	struct ModelInfo
	{
		ResourceWrapper<MeshCollection> mesh;
		std::map<int, ResourceWrapper<Material>> materials;
		std::vector<ResourceWrapper<Texture>> textures;
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
	void loadModelFromFile(const AssetInfo& aInfo, ModelImporter::ModelInfo& modelInfo);

	bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo);

	const LastImportedMaterials& getLastImportedMaterial() const;

private:
	friend class Engine;

	void loadModelFromAssimpScene(const aiScene* scene, const AssetInfo& aInfo, ModelImporter::ModelInfo& modelInfo);
	void processNode(const aiScene* aiScene, aiNode* aiNode);
	std::shared_ptr<Mesh> processMesh(const aiScene* aiScene, aiMesh* aiMesh);
	AssetWrapper<Texture> copyAiMaterialTexture(const aiScene* scene, aiMaterial* mat, aiTextureType type, const std::string& dir, std::unordered_map<std::string, AssetWrapper<Texture>>& cachedTextures, const AssetInfo& aInfo);
	static Texture::TextureType getTextureType(aiTextureType type);
private:
	//std::unordered_map<std::string, std::weak_ptr<Texture>> m_texturesCache;
	ModelLoadSession m_currentSession;
	std::shared_ptr<Assimp::Importer> m_importer = nullptr;

	std::string m_lastLoadedSceneName;
	LastImportedMaterials m_lastImportedMaterials;
};
