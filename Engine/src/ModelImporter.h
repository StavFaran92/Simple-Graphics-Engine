#pragma once

#include <unordered_map>
#include <map>
#include <memory>
#include <string>

#include "Core.h"
#include "Mesh.h"
#include "Texture.h"
#include "Entity.h"
#include "MeshBuilder.h"
#include "Material.h"
#include "MeshCollection.h"

// Forward declerations
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
class Engine;
class TextureHandler;
class Scene;
template<typename T>class ObjectHandler;

namespace Assimp
{
	class Importer;
}

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

	struct ModelImportSettings
	{
		std::string name;
		bool isTransient = false;
	};

	struct ModelInfo
	{
		Resource<MeshCollection> mesh;
		std::map<int, std::shared_ptr<Material>> materials;
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
	ModelImporter::ModelInfo import(const std::string& path, const ModelImportSettings& settings = {});

	/**
	 * Import a model from a file.
	 *
	 * \param path			path to the given file
	 * \param flipTexture	should flip loaded texture
	 * \return A poitner to the newly created model
	 */
	ModelInfo loadModelFromFile(const std::string& path, ModelInfo& modelInfo);

	void loadModelFromAssimpScene(const aiScene* scene, const std::string& path, ModelImporter::ModelInfo& modelInfo);
private:
	friend class Engine;

	void processNode(aiNode* node, const aiScene* scene, ModelImportSession& session);
	void processMesh(aiMesh* mesh, const aiScene* scene, ModelImportSession& session);
	Resource<Texture> importAiMaterialTexture(aiMaterial* mat, aiTextureType type, const std::string& dir);
	static Texture::TextureType getTextureType(aiTextureType type);
private:
	//std::unordered_map<std::string, std::weak_ptr<Texture>> m_texturesCache;
	std::map<uint32_t, ModelImportSession> m_sessions;
	std::shared_ptr<Assimp::Importer> m_importer = nullptr;

	std::string m_lastLoadedSceneName;
};
