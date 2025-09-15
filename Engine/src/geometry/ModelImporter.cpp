#include "geometry/ModelImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <algorithm>

#include "core/Logger.h"
#include <filesystem>
#include "geometry/MeshBuilder.h"
#include "memory/Resource.h"
#include "render/Material.h"
#include "runtime/Entity.h"
#include "component/Component.h"
#include "render/Shader.h"
#include "core/Engine.h"
#include "core/CacheSystem.h"
#include "runtime/Scene.h"
#include "memory/Assets.h"
#include "utils/AssimpGLMHelpers.h"
#include "core/Factory.h"
#include "geometry/MeshExporter.h"
#include "geometry/ShapeFactory.h"

bool findFile(const std::filesystem::path& directory, const std::string& fileName, std::filesystem::path& outputPath)
{
	for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
	{
		if (entry.is_regular_file() && entry.path().filename() == fileName)
		{
			outputPath = entry.path();  // Return the path if the file is found
			return true;
		}
	}
	return false;  // Return an empty optional if the file is not found
}

std::string findTexture(aiString str, const std::string& dir)
{
	std::filesystem::path filenamePath(str.C_Str());
	std::string filename = filenamePath.filename().string();

	std::string path = dir + "/" + filename;

	if (!std::filesystem::exists(path))
	{
		std::filesystem::path outputPath;
		if (!findFile(dir, filename, outputPath))
		{
			logWarning("Could not find texture: " + std::string(filename));
			return "";
		}
		path = outputPath.string();

	}

	return path;
}

ModelImporter::ModelImporter()
{
	Engine::get()->registerSubSystem<ModelImporter>(this);

	m_importer = std::make_shared<Assimp::Importer>();

	m_importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

	logInfo("Model importer init successfully.");
}

void ModelImporter::loadModelFromAssimpScene(const aiScene* scene, AssetInfo& aInfo, ModelImporter::ModelInfo& modelInfo)
{
	std::string modelName = std::filesystem::path(aInfo.filePath).filename().stem().string();

	// create new model session
	ModelImporter::ModelImportSession session;
	session.filepath = aInfo.filePath;
	session.fileDir = std::filesystem::path(aInfo.filePath).parent_path().string();
	session.name = modelName;
	session.mesh = modelInfo.mesh;

	// extract mesh from root node
	processNode(scene->mRootNode, scene, session);

	if (scene->HasMaterials())
	{
		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			auto& aMaterial = scene->mMaterials[i];

			std::string matName = aMaterial->GetName().C_Str();
			UUID uuid = Engine::get()->getMemoryManagementSystem()->getAssociation(matName);
			Resource<Material> material = Resource<Material>(uuid);
			modelInfo.materials[i] = material;
		}
	}
}

void ModelImporter::loadModelFromFile(AssetInfo& aInfo, ModelImporter::ModelInfo& modelInfo)
{
	std::string filepath;
	if (aInfo.isTransient)
	{
		filepath = aInfo.filePath;
	}
	else
	{
		filepath = Engine::get()->getProjectDirectory() + aInfo.filePath;
	}

	if (!std::filesystem::exists(filepath))
	{
		logError("File doesn't exists: " + filepath);
		return;
	}

	const aiScene* scene = nullptr;

	// If the scene was previously loaded last, we can optimize the load since it is already in memory.
	if (filepath == m_lastLoadedSceneName)
	{
		scene = m_importer->GetScene();
	}
	else
	{

		// read scene from file
		scene = m_importer->ReadFile(filepath, aiProcess_ValidateDataStructure);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			logError("ERROR::ASSIMP::{}", m_importer->GetErrorString());
			return;
		}
	}

	m_lastLoadedSceneName = filepath;

	loadModelFromAssimpScene(scene, aInfo, modelInfo);
}

bool ModelImporter::copyFiles(const std::string& fileLocation, AssetInfo& aInfo)
{
	auto fileDir = std::filesystem::path(fileLocation).parent_path().string();

	// read scene from file
	const aiScene* scene = m_importer->ReadFile(fileLocation,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace |
		aiProcess_ValidateDataStructure);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		logError("ERROR::ASSIMP::{}", m_importer->GetErrorString());
		return false;
	}

	std::unordered_map<std::string, Resource<Texture>> cachedTextures;

	// Import materials and textures
	if (scene->HasMaterials())
	{
		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			auto& aMaterial = scene->mMaterials[i];

			// get uuid using tex name from association map
			AssetInfo materialAssetInfo;
			materialAssetInfo.isTransient = aInfo.isTransient;
			materialAssetInfo.assetDirectory = aInfo.assetDirectory;
			materialAssetInfo.name = aMaterial->GetName().C_Str();
			auto& material = Material::create(materialAssetInfo);
			Engine::get()->getMemoryManagementSystem()->addAssociation(aMaterial->GetName().C_Str(), material.getUID());
			material->setName(aMaterial->GetName().C_Str());

			auto& diffuse = copyAiMaterialTexture(aMaterial, aiTextureType::aiTextureType_DIFFUSE, fileDir, cachedTextures, aInfo);
			if (!diffuse.isEmpty())
			{
				material->setTexture(Texture::TextureType::Albedo, diffuse);
			}

			auto& normal = copyAiMaterialTexture(aMaterial, aiTextureType::aiTextureType_NORMALS, fileDir, cachedTextures, aInfo);
			if (!normal.isEmpty())
			{
				material->setTexture(Texture::TextureType::Normal, normal);
			}

			Material::save(material);
		}
	}

	Resource<MeshCollection> mesh = Factory<MeshCollection>::create();

	if (scene->HasTextures())
	{
		aiScene* strippedScene = new aiScene(*scene);

		for (unsigned int i = 0; i < strippedScene->mNumMaterials; ++i)
		{
			aiMaterial* mat = strippedScene->mMaterials[i];

			for (int t = aiTextureType_NONE + 1; t <= aiTextureType_UNKNOWN; ++t)
			{
				aiTextureType texType = static_cast<aiTextureType>(t);

				unsigned int texCount = mat->GetTextureCount(texType);
				for (unsigned int index = 0; index < texCount; ++index)
				{
					strippedScene->mTextures[index] = nullptr;
					strippedScene->mNumTextures = 0;
					// Remove only the texture reference (path binding)
					//mat->RemoveProperty(AI_MATKEY_TEXTURE(texType, index));
				}
			}
		}

		scene = strippedScene;
	}

	// TODO I should probably copy the file instead of export (issue with GLTF and bin)
	MeshExporter::exportMesh(aInfo.name, aInfo.assetDirectory, scene);

	return true;
}

void ModelImporter::processNode(aiNode* node, const aiScene* scene, ModelImporter::ModelImportSession& session)
{
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		processMesh(scene->mMeshes[node->mMeshes[i]], scene, session);
	}

	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, session);
	}
}

struct BoneWeight
{
	unsigned int boneID = -1;
	float weight = 0.f;
};

void ModelImporter::processMesh(aiMesh* mesh, const aiScene* scene, ModelImporter::ModelImportSession& session)
{
	MeshBuilder builder;

	std::shared_ptr<Mesh> generatedMesh = std::make_shared<Mesh>();

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec2> texcoords;
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		// process vertex positions, normals and texture coordinates
		if (mesh->HasPositions())
		{
			glm::vec3 pos;
			pos.x = mesh->mVertices[i].x;
			pos.y = mesh->mVertices[i].y;
			pos.z = mesh->mVertices[i].z;
			positions.emplace_back(pos);
		}

		if (mesh->HasNormals())
		{
			glm::vec3 normal;
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;
			normals.emplace_back(normal);
		}

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			texcoords.emplace_back(vec);
		}
		else
		{
			texcoords.emplace_back(glm::vec2(0.0f, 0.0f));
		}

		if (mesh->HasTangentsAndBitangents())
		{
			tangents.emplace_back(glm::vec3{ mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z });
		}
	}
	// process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	
	
	if (mesh->HasBones())
	{
		std::vector<glm::ivec3> bonesIDs;
		std::vector<glm::vec3> bonesWeights;

		// Will be used by mesh as base array for bone transformations uniform
		std::vector<glm::mat4> bonesOffsets;

		// an intermediate helper map used to facilitate in extracting bone weight for each vertex
		std::map<int, std::map<float, BoneWeight>> vertexToBoneMap;

		// Extract Bone to ID map 
		auto& boneNameToIDMap = session.boneNameToIDMap;
		auto& boneCount = session.boneCount;

		// Iterate all bones in Assimp model
		for (int i = 0; i < mesh->mNumBones; i++)
		{
			auto bone = mesh->mBones[i];
			auto boneName = bone->mName.C_Str();

			// a new bone is found, increment bone ID and add bone offset to offsets array
			if (boneNameToIDMap.find(boneName) == boneNameToIDMap.end())
			{
				boneNameToIDMap[boneName] = boneCount++;
				bonesOffsets.push_back(AssimpGLMHelpers::convertMat4ToGLMFormat(bone->mOffsetMatrix));
			}

			// Extract weights for each vertex, we use a map container and a (-weight) key so the entries will be sorted in descending order when we iterate it
			int numOfWeights = bone->mNumWeights;
			auto weights = bone->mWeights;
			for (int j = 0; j < numOfWeights; j++)
			{
				unsigned int vertexID = weights[j].mVertexId;
				float weight = weights[j].mWeight;
				vertexToBoneMap[vertexID][-weight] = BoneWeight{ boneNameToIDMap[boneName] , weight };
			}
		}
		
		bonesIDs.resize(vertexToBoneMap.size(), glm::ivec3(- 1));
		bonesWeights.resize(vertexToBoneMap.size());

		// post process influence data in bone info map, we discard the least influential bone weights
		for (auto& [vID, boneInfluenceMap] : vertexToBoneMap)
		{
			// TODO resize and normalize
			//if (boneInfluenceMap.size() > 3)
			//{
			//	boneInfluenceMap.resize(3);
			//}
			auto boneIter = boneInfluenceMap.begin();
			int index = 0;
			while (boneIter != boneInfluenceMap.end() && index < 3)
			{
				bonesIDs[vID][index] = (int)(*boneIter).second.boneID;
				bonesWeights[vID][index] = (*boneIter).second.weight;
				boneIter++;
				index++;
			}
		}

		builder.addBoneIDs(bonesIDs)
			.addBoneWeights(bonesWeights);

		session.mesh.get()->addBonesInfo(bonesOffsets, boneNameToIDMap);
	}

	builder.setMaterialIndex(mesh->mMaterialIndex);

	builder.addPositions(positions)
		.addNormals(normals)
		.addTexcoords(texcoords)
		.addIndices(indices)
		.addTangents(tangents);

	// build mesh
	builder.build(*generatedMesh.get());

	session.mesh.get()->addMesh(generatedMesh);
}



Resource<Texture> ModelImporter::copyAiMaterialTexture(aiMaterial* mat, aiTextureType type, const std::string& dir, std::unordered_map<std::string, Resource<Texture>>& cachedTextures, AssetInfo& aInfo)
{
	aiString str;
	if (mat->GetTexture(type, 0, &str) != aiReturn_SUCCESS)
	{
		return Resource<Texture>::empty;
	}

	std::string path = findTexture(str, dir);
	if (path.empty())
	{
		return Resource<Texture>::empty;
	}

	if (cachedTextures.find(path) != cachedTextures.end())
	{
		// Already loaded
		return cachedTextures[path];
	}

	Texture::TextureImportSettings tSettings;
	tSettings.targetDirectory = aInfo.assetDirectory;
	tSettings.isTransient = aInfo.isTransient;
	auto texture = Texture::import(path, tSettings);

	cachedTextures.insert({ path, texture });

	return texture;
}

Texture::TextureType ModelImporter::getTextureType(aiTextureType type)
{
	switch (type)
	{
	case aiTextureType::aiTextureType_DIFFUSE:
		return Texture::TextureType::Diffuse;
	case aiTextureType::aiTextureType_SPECULAR:
		return Texture::TextureType::Specular;
	case aiTextureType::aiTextureType_HEIGHT:
		return Texture::TextureType::Normal;
	default:
		logError("Unsupported type: " + type);
		return Texture::TextureType::None;
	}
}