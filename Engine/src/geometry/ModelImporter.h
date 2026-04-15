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
#include "systems/SubSystem.h"

// Forward declerations
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;
class Engine;
class TextureHandler;
class Model;
class ModelLoadDescriptor;
class Scene;
template<typename T>class ResourceRef;

namespace Assimp
{
	class Importer;
}

class EngineAPI ModelImporter : public SubSystem
{
public:

	struct ModelInfo
	{
		std::vector<MeshData> meshDataList;
		std::vector<MaterialData> materialDataList;
		std::vector<TextureData> textureDataList;
		std::vector<glm::mat4> bonesOffsets;
		std::unordered_map<std::string, unsigned int> bonesNameToIDMap;

		std::unordered_set<std::string> textureFilepathList;
		std::unordered_map<std::string, std::map<std::string, std::string>> materialToTextureMap;

	};

	struct ModelParseSession
	{
		std::string filepath;
		std::string fileDir;
		std::string name;
		int nodeIndex = 0;
		int childIndex = 0;
		Entity root;
		//std::unordered_map<std::string, unsigned int> boneNameToIDMap;
		unsigned int boneCount = 0;
		ModelInfo modelInfo;
	};

	//struct LastImportedMaterials
	//{
	//	std::map<int, MaterialAssetRef> materials;
	//};

	

	//struct ModelInfo
	//{
	//	ResourceWrapper<MeshGroup> mesh;
	//	std::map<int, MaterialResourceRef> materials;
	//	std::vector<TextureResourceRef> textures;

	//	std::map<std::string, std::string> materialInfo;
	//};

	/** Constructor */
	ModelImporter();

	/**
	 * Import a model from a file.
	 *
	 * \param path			path to the given file
	 * \param flipTexture	should flip loaded texture
	 * \return A poitner to the newly created model
	 */
	//void loadModelFromFile(const ModelLoadDescriptor& desc, ModelImporter::ModelInfo& modelInfo);

	

	bool parseModel(const std::string& fileLocation, ModelImporter::ModelInfo& outModelInfo);

	//bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo);

	//const LastImportedMaterials& getLastImportedMaterial() const;

private:
	void loadModelFromAssimpScene(const aiScene* scene, ModelImporter::ModelParseSession& session);

	void processNode(const aiScene* aiScene, aiNode* aiNode, ModelImporter::ModelParseSession& session);

	MeshData processMesh(const aiScene* aiScene, aiMesh* aiMesh, ModelImporter::ModelParseSession& session);

	void parseAiMaterials(const aiScene* scene, ModelImporter::ModelParseSession& session);

	void parseAiTexture(const aiScene* scene,
		aiMaterial* mat, 
		aiTextureType type, 
		std::unordered_set<std::string>& cachedTextureNames,
		MaterialData& materialData,
		ModelImporter::ModelParseSession& session);
private:
	//std::unordered_map<std::string, std::weak_ptr<Texture>> m_texturesCache;
	//ModelLoadSession m_currentSession;
	std::shared_ptr<Assimp::Importer> m_importer = nullptr;

	std::string m_lastLoadedSceneName;
	//LastImportedMaterials m_lastImportedMaterials;
};
