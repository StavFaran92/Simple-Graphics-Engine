#include "geometry/ModelImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "core/Logger.h"
#include <filesystem>
#include "geometry/MeshBuilder.h"
#include "memory/ResourceRef.h"
#include "render/Material.h"
#include "component/Component.h"
#include "render/Shader.h"
#include "core/Engine.h"
#include "memory/Assets.h"
#include "utils/AssimpGLMHelpers.h"
#include "core/Factory.h"
#include <GL/glew.h>
#include "utils/STBIHelper.h"
#include "texture/Texture.h"
#include "geometry/Model.h"
#include "render/MaterialDataParser.h"

#include "Utils/MikkTSpaceImpl.h"

void extractAiMaterialProperties(const aiMaterial* aiMat, MaterialData& materialData)
{
	if (!aiMat)
	{
		logError("Null material.");
		return;
	}

	ai_real opacityFactor;
	if (aiMat->Get(AI_MATKEY_OPACITY, opacityFactor) == aiReturn_SUCCESS)
	{
		if (opacityFactor < 1.f)
		{
			materialData.setMaterialRenderMode(MaterialRenderMode::Transparent);
			materialData.setUniform(SHADER_PROPERTY_PBR_OPACITY_FACTOR, opacityFactor);
			//mat->setUniformValue(SHADER_PROPERTY_PBR_OPACITY_FACTOR, opacityFactor);
		}

	}

	aiColor3D diffuseColor;
	if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == aiReturn_SUCCESS)
	{
		//mat->setUniformValue(SHADER_PROPERTY_PBR_COLOR_DIFFUSE, glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b));
		materialData.setUniform(SHADER_PROPERTY_PBR_COLOR_DIFFUSE, glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b));
	}

	ai_real rounghnessFactor;
	if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, rounghnessFactor) == aiReturn_SUCCESS)
	{
		//mat->setUniformValue(SHADER_PROPERTY_PBR_ROUGHNESS_FACTOR, rounghnessFactor);
		materialData.setUniform(SHADER_PROPERTY_PBR_ROUGHNESS_FACTOR, rounghnessFactor);
	}

	ai_real metallicFactor;
	if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == aiReturn_SUCCESS)
	{
		//mat->setUniformValue(SHADER_PROPERTY_PBR_METALLIC_FACTOR, metallicFactor);
		materialData.setUniform(SHADER_PROPERTY_PBR_METALLIC_FACTOR, metallicFactor);
	}


}

void PrintMaterialProperties(const aiMaterial* mat) {
	if (!mat) {
		std::cout << "Null material\n";
		return;
	}

	std::cout << "Material has " << mat->mNumProperties << " properties\n";

	for (unsigned int i = 0; i < mat->mNumProperties; i++) {
		aiMaterialProperty* prop = mat->mProperties[i];
		std::cout << "----------------------------------------\n";
		std::cout << "Key: " << prop->mKey.C_Str()
			<< " | Type: " << prop->mType
			<< " | Semantic: " << prop->mSemantic
			<< " | Index: " << prop->mIndex
			<< " | Size: " << prop->mDataLength << "\n";

		// Try to pretty-print depending on type
		if (prop->mType == aiPTI_String) {
			aiString str;
			memcpy(&str, prop->mData, sizeof(aiString));
			std::cout << "  Value (string): " << str.C_Str() << "\n";
		}
		else if (prop->mType == aiPTI_Float) {
			unsigned count = prop->mDataLength / sizeof(float);
			float* vals = (float*)prop->mData;
			std::cout << "  Value (float[" << count << "]): ";
			for (unsigned c = 0; c < count; c++)
				std::cout << vals[c] << " ";
			std::cout << "\n";
		}
		else if (prop->mType == aiPTI_Integer) {
			unsigned count = prop->mDataLength / sizeof(int);
			int* vals = (int*)prop->mData;
			std::cout << "  Value (int[" << count << "]): ";
			for (unsigned c = 0; c < count; c++)
				std::cout << vals[c] << " ";
			std::cout << "\n";
		}
		else if (prop->mType == aiPTI_Buffer) {
			std::cout << "  Value: (raw buffer, " << prop->mDataLength << " bytes)\n";
		}
		else {
			std::cout << "  Value: (unknown type)\n";
		}
	}
}

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

void ModelImporter::parseAiMaterials(const aiScene* scene, ModelImporter::ModelParseSession& session)
{
	std::unordered_set<std::string> cachedTextures;

	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial* aMaterial = scene->mMaterials[i];
		std::string materialName = std::string(aMaterial->GetName().C_Str());

		MaterialData materialData;
		materialData.name = materialName;
		materialData.setMaterialRenderMode(MaterialRenderMode::Opaque);

		extractAiMaterialProperties(aMaterial, materialData);

		parseAiTexture(scene, aMaterial, aiTextureType::aiTextureType_DIFFUSE, cachedTextures, materialData, session);
		parseAiTexture(scene, aMaterial, aiTextureType::aiTextureType_NORMALS, cachedTextures, materialData, session);
		parseAiTexture(scene, aMaterial, aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, cachedTextures, materialData, session);
		parseAiTexture(scene, aMaterial, aiTextureType::aiTextureType_METALNESS, cachedTextures, materialData, session);
		parseAiTexture(scene, aMaterial, aiTextureType::aiTextureType_AMBIENT_OCCLUSION, cachedTextures, materialData, session);

		session.modelInfo.materialDataList.push_back(materialData);
	}
}

void ModelImporter::loadModelFromAssimpScene(const aiScene* scene, ModelImporter::ModelParseSession& session)
{
	// extract mesh from root node
	processNode(scene, scene->mRootNode, session);

	// Import materials and textures
	if (scene->HasMaterials())
	{
		parseAiMaterials(scene, session);

		
	}
}

//void ModelImporter::loadModelFromFile(const ModelLoadDescriptor& resourceDesc, ModelImporter::ModelInfo& modelInfo)
//{
//	std::string filepath = resourceDesc.sourcePath;
//
//	if (!std::filesystem::exists(filepath))
//	{
//		logError("File doesn't exists: " + filepath);
//		return;
//	}
//
//	const aiScene* scene = nullptr;
//
//	// If the scene was previously loaded last, we can optimize the load since it is already in memory.
//	if (filepath == m_lastLoadedSceneName)
//	{
//		scene = m_importer->GetScene();
//	}
//	else
//	{
//
//		// read scene from file
//		scene = m_importer->ReadFile(filepath, aiProcess_Triangulate |
//			aiProcess_GenSmoothNormals |
//			aiProcess_FlipUVs |
//			//aiProcess_CalcTangentSpace |
//			aiProcess_ValidateDataStructure);
//
//		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
//		{
//			logError("ERROR::ASSIMP::{}", m_importer->GetErrorString());
//			return;
//		}
//	}
//
//	m_lastLoadedSceneName = filepath;
//
//	std::string modelName = std::filesystem::path(filepath).filename().stem().string();
//
//	// create new model session
//	ModelImporter::ModelLoadSession session;
//	session.filepath = filepath;
//	session.fileDir = std::filesystem::path(filepath).parent_path().string();
//	session.name = modelName;
//	session.mesh = modelInfo.mesh;
//
//	m_currentSession = session;
//
//	loadModelFromAssimpScene(scene, modelInfo);
//}

bool ModelImporter::parseModel(const std::string& filepath, ModelImporter::ModelInfo& outModelInfo)
{
	// Validate
	if (!std::filesystem::exists(filepath))
	{
		logError("File doesn't exists: " + filepath);
		return false;
	}

	// If the scene was previously loaded last, we can optimize the load since it is already in memory.
	const aiScene* scene = nullptr;
	if (filepath == m_lastLoadedSceneName)
	{
		scene = m_importer->GetScene();
	}
	else
	{

		// read scene from file
		scene = m_importer->ReadFile(filepath, aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			//aiProcess_CalcTangentSpace |
			aiProcess_ValidateDataStructure);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			logError("ERROR::ASSIMP::{}", m_importer->GetErrorString());
			return false;
		}
	}

	m_lastLoadedSceneName = filepath;

	std::string modelName = std::filesystem::path(filepath).filename().stem().string();

	// create new model session
	ModelImporter::ModelParseSession session;
	session.filepath = filepath;
	session.fileDir = std::filesystem::path(filepath).parent_path().string();
	session.name = modelName;

	loadModelFromAssimpScene(scene, session);

	outModelInfo = session.modelInfo;

	return true;
}

void ModelImporter::processNode(const aiScene* scene, aiNode* node, ModelImporter::ModelParseSession& session)
{
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		MeshData meshData = processMesh(scene, scene->mMeshes[node->mMeshes[i]], session);

		aiMatrix4x4 transform = node->mTransformation;
		aiNode* parent = node->mParent;
		while (parent)
		{
			transform = parent->mTransformation * transform;
			parent = parent->mParent;
		}

		meshData.restTransform = AssimpGLMHelpers::convertMat4ToGLMFormat(transform);
		session.modelInfo.meshDataList.push_back(meshData);
	}

	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(scene, node->mChildren[i], session);
	}
}

struct BoneWeight
{
	unsigned int boneID = -1;
	float weight = 0.f;
};

MeshData ModelImporter::processMesh(const aiScene* aiScene, aiMesh* aiMesh, ModelImporter::ModelParseSession& session)
{
	//MeshBuilder builder;

	//std::shared_ptr<Mesh> generatedMesh = std::make_shared<Mesh>();

	MeshData meshData;
	meshData.name = aiMesh->mName.C_Str();

	//generatedMesh->setName(aiMesh->mName.C_Str());

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec4> tangents;
	std::vector<glm::vec2> texcoords;
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
	{
		// process vertex positions, normals and texture coordinates
		if (aiMesh->HasPositions())
		{
			glm::vec3 pos;
			pos.x = aiMesh->mVertices[i].x;
			pos.y = aiMesh->mVertices[i].y;
			pos.z = aiMesh->mVertices[i].z;
			positions.emplace_back(pos);
		}

		if (aiMesh->HasNormals())
		{
			glm::vec3 normal;
			normal.x = aiMesh->mNormals[i].x;
			normal.y = aiMesh->mNormals[i].y;
			normal.z = aiMesh->mNormals[i].z;
			normals.emplace_back(normal);
		}

		if (aiMesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = aiMesh->mTextureCoords[0][i].x;
			vec.y = aiMesh->mTextureCoords[0][i].y;
			texcoords.emplace_back(vec);
		}
		else
		{
			texcoords.emplace_back(glm::vec2(0.0f, 0.0f));
		}

		if (aiMesh->HasTangentsAndBitangents())
		{
			glm::vec3 T(aiMesh->mTangents[i].x, aiMesh->mTangents[i].y, aiMesh->mTangents[i].z);
			glm::vec3 B(aiMesh->mBitangents[i].x, aiMesh->mBitangents[i].y, aiMesh->mBitangents[i].z);
			glm::vec3 N(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);

			float sign = (glm::dot(glm::cross(N, T), B) < 0.0f) ? -1.0f : 1.0f;
			tangents.emplace_back(glm::vec4{ aiMesh->mTangents[i].x, aiMesh->mTangents[i].y, aiMesh->mTangents[i].z, sign });
		}
	}
	// process indices
	for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
	{
		aiFace face = aiMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	
	
	if (aiMesh->HasBones())
	{
		std::vector<glm::ivec3> bonesIDs;
		std::vector<glm::vec3> bonesWeights;

		// Will be used by mesh as base array for bone transformations uniform
		std::vector<glm::mat4> bonesOffsets;

		// an intermediate helper map used to facilitate in extracting bone weight for each vertex
		std::map<int, std::map<float, BoneWeight>> vertexToBoneMap;

		// Extract Bone to ID map 
		auto& boneNameToIDMap = session.modelInfo.bonesNameToIDMap;
		auto& boneCount = session.boneCount;

		// Iterate all bones in Assimp model
		for (int i = 0; i < aiMesh->mNumBones; i++)
		{
			auto bone = aiMesh->mBones[i];
			auto boneName = bone->mName.C_Str();

			// a new bone is found, increment bone ID and add bone offset to offsets array
			if (boneNameToIDMap.find(boneName) == boneNameToIDMap.end())
			{
				unsigned int boneID = boneCount++;
				boneNameToIDMap[boneName] = boneID;
				glm::mat4 offset = AssimpGLMHelpers::convertMat4ToGLMFormat(bone->mOffsetMatrix);
				bonesOffsets.push_back(offset);
				
				// Accumulate bone info into ModelInfo
				session.modelInfo.bonesNameToIDMap[boneName] = boneID;
				session.modelInfo.bonesOffsets.push_back(offset);
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

		meshData.bonesIDs.insert(meshData.bonesIDs.end(), bonesIDs.begin(), bonesIDs.end());
		meshData.bonesWeights.insert(meshData.bonesWeights.end(), bonesWeights.begin(), bonesWeights.end());
	}

	meshData.materialIndex = aiMesh->mMaterialIndex;
	meshData.m_positions = positions;
	meshData.m_normals = normals;
	meshData.m_texCoords = texcoords;
	meshData.m_indices = indices;
	meshData.m_tangents = tangents;

	return meshData;
}


std::string convertAiTextureTypeToSGETextureType(aiTextureType type)
{
	switch (type)
	{
	case aiTextureType::aiTextureType_DIFFUSE:
		return SHADER_PROPERTY_PBR_SAMPLER_ALBEDO;
		case aiTextureType::aiTextureType_NORMALS:
		return SHADER_PROPERTY_PBR_SAMPLER_NORMAL;
	case aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS:
		return SHADER_PROPERTY_PBR_SAMPLER_ROUGHNESS;
	case aiTextureType::aiTextureType_METALNESS:
		return SHADER_PROPERTY_PBR_SAMPLER_METALLIC;
	case aiTextureType::aiTextureType_AMBIENT_OCCLUSION:
		return SHADER_PROPERTY_PBR_SAMPLER_AO;
	}
	return "";
}

void ModelImporter::parseAiTexture(const aiScene* scene,
	aiMaterial* mat, 
	aiTextureType type, 
	std::unordered_set<std::string>& cachedTextureNames, 
	MaterialData& materialData,
	ModelImporter::ModelParseSession& session)
{
	std::string sgeTextureType = convertAiTextureTypeToSGETextureType(type);

	aiString str;
	if (mat->GetTexture(type, 0, &str) != aiReturn_SUCCESS)
	{
		return;
	}

	const aiTexture* aiTexture = scene->GetEmbeddedTexture(str.C_Str());
	if(aiTexture)
	{
		//Embedded texture found, extract it.
		std::string textureName = std::filesystem::path(aiTexture->mFilename.C_Str()).filename().stem().string();

		if (!textureName.empty())
		{
			if (cachedTextureNames.find(textureName) != cachedTextureNames.end())
			{
				// Already loaded
				session.modelInfo.materialToTextureMap[materialData.name][sgeTextureType] = textureName;
				return;
			}
		}


		int width = 0;
		int height = 0;
		int channels = 0;
		unsigned char* pixelData = nullptr;
		if (aiTexture->mHeight == 0)
		{
			// Compressed image (PNG/JPG) in memory
			size_t size = aiTexture->mWidth;
			const unsigned char* data = reinterpret_cast<unsigned char*>(aiTexture->pcData);

			
			pixelData = STBIHelper::decodeCompressedFromMemory(data, size, &width, &height, &channels);
		}
		else {
			// Raw ARGB8888 pixels
			width = aiTexture->mWidth;
			height = aiTexture->mHeight;
			pixelData = reinterpret_cast<unsigned char*>(aiTexture->pcData);
		}

		TextureSemantic usage = TextureSemantic::Color;
		if (type == aiTextureType_DIFFUSE || 
			type == aiTextureType_DIFFUSE_ROUGHNESS || 
			type == aiTextureType_METALNESS || 
			type == aiTextureType_AMBIENT_OCCLUSION) // todo fix
		{
			usage = TextureSemantic::Color;
		}
		else if (type == aiTextureType_NORMALS)
		{
			usage = TextureSemantic::Normal;
		}

		TextureData textureData;
		textureData.width = width;
		textureData.height = height;
		textureData.channels = channels;
		textureData.internalFormat = Texture::getInternalFormatFromUsage(usage);
		textureData.format = Texture::getFormatFromChannels(channels);
		textureData.type = TextureType::UNSIGNED_BYTE;
		textureData.filter = TextureFilter::Linear;
		textureData.wrap = TextureWrap::Repeat;
		textureData.data = ImageBuffer{ pixelData, (size_t)width * height * channels };
		textureData.textureName = textureName;

		if (!textureName.empty())
		{
			cachedTextureNames.insert(textureName);
			session.modelInfo.textureDataList.push_back(textureData);
			session.modelInfo.materialToTextureMap[materialData.name][sgeTextureType] = textureName;
		}
	}	
	else
	{
		std::string path = findTexture(str, session.fileDir);
		if (path.empty())
		{
			return;
		}

		if (cachedTextureNames.find(path) != cachedTextureNames.end())
		{
			// Already loaded
			session.modelInfo.materialToTextureMap[materialData.name][sgeTextureType] = path;
			return;
		}

		session.modelInfo.textureFilepathList.insert(path);
		session.modelInfo.materialToTextureMap[materialData.name][sgeTextureType] = path;
	}

}