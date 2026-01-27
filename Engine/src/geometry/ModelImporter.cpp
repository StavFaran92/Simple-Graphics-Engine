#include "geometry/ModelImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "core/Logger.h"
#include <filesystem>
#include "geometry/MeshBuilder.h"
#include "memory/ResourceWrapper.h"
#include "render/Material.h"
#include "component/Component.h"
#include "render/Shader.h"
#include "core/Engine.h"
#include "memory/Assets.h"
#include "utils/AssimpGLMHelpers.h"
#include "core/Factory.h"
#include <GL/glew.h>
#include "utils/STBIHelper.h"
#include "geometry/MeshGroup.h"

#include "Utils/MikkTSpaceImpl.h"

void extractAiMaterialProperties(const aiMaterial* aiMat, ResourceWrapper<Material>& mat)
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
			mat->setMaterialRenderMode(MaterialRenderMode::Transparent);
			mat->setUniformValue(SHADER_PROPERTY_PBR_OPACITY_FACTOR, opacityFactor);
		}

	}

	aiColor3D diffuseColor;
	if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == aiReturn_SUCCESS)
	{
		mat->setUniformValue(SHADER_PROPERTY_PBR_COLOR_DIFFUSE, glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b));
	}

	ai_real rounghnessFactor;
	if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, rounghnessFactor) == aiReturn_SUCCESS)
	{
		mat->setUniformValue(SHADER_PROPERTY_PBR_ROUGHNESS_FACTOR, rounghnessFactor);
	}

	ai_real metallicFactor;
	if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == aiReturn_SUCCESS)
	{
		mat->setUniformValue(SHADER_PROPERTY_PBR_METALLIC_FACTOR, metallicFactor);
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

void ModelImporter::loadModelFromAssimpScene(const aiScene* scene, const AssetRecord& aInfo, ModelImporter::ModelInfo& modelInfo)
{
	std::string modelName = std::filesystem::path(aInfo.relativefilePath).filename().stem().string();

	// create new model session
	ModelImporter::ModelLoadSession session;
	session.filepath = aInfo.relativefilePath;
	session.fileDir = std::filesystem::path(aInfo.relativefilePath).parent_path().string();
	session.name = modelName;
	session.mesh = modelInfo.mesh;

	m_currentSession = session;

	// extract mesh from root node
	processNode(scene, scene->mRootNode);

	if (scene->HasMaterials())
	{
		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			auto& aMaterial = scene->mMaterials[i];

			//PrintMaterialProperties(aMaterial);

			for (const auto& [key, value] : aInfo.createDescriptor.attributes)
			{
				// Look for _MAT_ in the attribute key
				const std::string tag = "_MAT_";
				size_t pos = key.find(tag);
				if (pos == std::string::npos)
					continue; // not a material attribute

				// Extract index: key format is NAME_MAT_X
				// so we read everything after "_MAT_"
				size_t indexPos = pos + tag.length();
				std::string indexStr = key.substr(indexPos);

				// Convert to integer safely
				int matIndex = -1;
				try {
					matIndex = std::stoi(indexStr);
				}
				catch (...) {
					logError("Invalid material index for attribute '{}'", key);
					continue;
				}

				// Convert attribute value to UUID
				UUID uuid;
				try {
					uuid = UUID(std::stoull(value));
				}
				catch (...) {
					logError("Invalid UUID for material '{}'", key);
					continue;
				}

				// Load material asset
				AssetHandle<MaterialAsset> material(uuid);
				modelInfo.materials[matIndex] = material.resource();

				logTrace("Assigned material index {} -> UUID {}", matIndex, uuid);
			}
		}
	}
}

void ModelImporter::loadModelFromFile(const MeshGroupLoadDescriptor& resourceDesc, ModelImporter::ModelInfo& modelInfo)
{
	std::string filepath = aInfo.fullFilePath;

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
		scene = m_importer->ReadFile(filepath, aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			//aiProcess_CalcTangentSpace |
			aiProcess_ValidateDataStructure);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			logError("ERROR::ASSIMP::{}", m_importer->GetErrorString());
			return;
		}
	}

	m_lastLoadedSceneName = filepath;

	loadModelFromAssimpScene(scene, aInfo, modelInfo);
}

bool ModelImporter::copyFiles(const std::string& fileLocation, AssetRecord& aInfo)
{
	std::string finalFilePath = fileLocation;

	// If GLTF2, search for corresponding .GLB file
	std::filesystem::path path(fileLocation);
	if (path.extension().generic_string() == ".gltf")
	{
		// Switch to GLB file if found
		auto parentPath = path.parent_path();
		auto filenameStem = path.stem(); // filename without extension

		// Construct potential .glb path
		std::filesystem::path glbPath = parentPath / (filenameStem.string() + ".glb");

		if (std::filesystem::exists(glbPath))
		{
			finalFilePath = glbPath.generic_string();
		}

		// Check for a .bin file with the same name
		std::filesystem::path binPath = parentPath / (filenameStem.string() + ".bin");
		if (std::filesystem::exists(binPath))
		{
			

			std::filesystem::path targetParentPath = std::filesystem::path(aInfo.fullFilePath).parent_path();
			std::filesystem::path targetBinFilePath = targetParentPath / (filenameStem.string() + ".bin");
			std::filesystem::copy_file(binPath, targetBinFilePath, std::filesystem::copy_options::overwrite_existing);

			logInfo("Found external .bin file: {}, copy into {}", binPath.generic_string(), targetBinFilePath.generic_string());
			// Use it as the buffer source
		}
	}

	std::filesystem::copy_file(finalFilePath, aInfo.fullFilePath, std::filesystem::copy_options::overwrite_existing);

	auto fileDir = std::filesystem::path(finalFilePath).parent_path().string();

	// read scene from file
	const aiScene* scene = m_importer->ReadFile(finalFilePath, aiProcess_ValidateDataStructure);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		logError("ERROR::ASSIMP::{}", m_importer->GetErrorString());
		return false;
	}

	m_lastImportedMaterials = LastImportedMaterials();

	std::unordered_map<std::string, AssetHandle<TextureAsset>> cachedTextures;

	// Import materials and textures
	if (scene->HasMaterials())
	{
		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			auto& aMaterial = scene->mMaterials[i];
			std::string materialName = std::string(aMaterial->GetName().C_Str());
			std::string materialID = aInfo.name + "_MAT_" + std::to_string(i);

			// get uuid using tex name from association map
			auto& material = Material::create(MaterialRenderMode::Opaque);

			extractAiMaterialProperties(aMaterial, material);
			
			//Engine::get()->getMemoryManagementSystem()->addAssociation(materialID, material.getUID());
			material->setName(materialName);

			auto& diffuse = copyAiMaterialTexture(scene, aMaterial, aiTextureType::aiTextureType_DIFFUSE, fileDir, cachedTextures, aInfo);
			if (!diffuse.isEmpty())
			{
				auto diffuseSampler = std::make_shared<TextureSampler>(3);
				diffuseSampler->texture = diffuse;
				material->setSampler(SHADER_PROPERTY_PBR_SAMPLER_ALBEDO, diffuseSampler);
				material->setSamplerEnabled(SHADER_PROPERTY_PBR_SAMPLER_ALBEDO, true);
			}

			auto& normal = copyAiMaterialTexture(scene, aMaterial, aiTextureType::aiTextureType_NORMALS, fileDir, cachedTextures, aInfo);
			if (!normal.isEmpty())
			{
				auto normalSampler = std::make_shared<TextureSampler>(3);
				normalSampler->texture = normal;
				material->setSampler(SHADER_PROPERTY_PBR_SAMPLER_NORMAL, normalSampler);
				material->setSamplerEnabled(SHADER_PROPERTY_PBR_SAMPLER_NORMAL, true);
			}

			auto& roughness = copyAiMaterialTexture(scene, aMaterial, aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, fileDir, cachedTextures, aInfo);
			if (!roughness.isEmpty())
			{
				auto roughnessSampler = std::make_shared<TextureSampler>(1);
				roughnessSampler->texture = roughness;
				roughnessSampler->channelMaskR = TextureSampler::Color::G;
				material->setSampler(SHADER_PROPERTY_PBR_SAMPLER_ROUGHNESS, roughnessSampler);
				material->setSamplerEnabled(SHADER_PROPERTY_PBR_SAMPLER_ROUGHNESS, true);
			}

			// Metallic map
			auto& metallic = copyAiMaterialTexture(scene, aMaterial, aiTextureType::aiTextureType_METALNESS, fileDir, cachedTextures, aInfo);
			if (!metallic.isEmpty())
			{
				auto metallicSampler = std::make_shared<TextureSampler>(1);
				metallicSampler->texture = metallic;
				metallicSampler->channelMaskR = TextureSampler::Color::B;
				material->setSampler(SHADER_PROPERTY_PBR_SAMPLER_METALLIC, metallicSampler);
				material->setSamplerEnabled(SHADER_PROPERTY_PBR_SAMPLER_METALLIC, true);
			}

			// Ambient Occlusion map
			auto& ao = copyAiMaterialTexture(scene, aMaterial, aiTextureType::aiTextureType_AMBIENT_OCCLUSION, fileDir, cachedTextures, aInfo);
			if (!ao.isEmpty())
			{
				auto aoSampler = std::make_shared<TextureSampler>(1);
				aoSampler->texture = ao;
				aoSampler->channelMaskR = TextureSampler::Color::R;
				material->setSampler(SHADER_PROPERTY_PBR_SAMPLER_AO, aoSampler);
				material->setSamplerEnabled(SHADER_PROPERTY_PBR_SAMPLER_AO, true);
			}

			

			AssetCreateDescriptor materialAssetInfo;
			materialAssetInfo.isEngineOwned = aInfo.isEngineOwned;
			materialAssetInfo.assetDirectory = aInfo.assetDirectory;
			materialAssetInfo.targetDirectory = aInfo.targetDirectory;
			materialAssetInfo.name = materialName;
			materialAssetInfo.aType = AssetType::MATERIAL;
			AssetHandle<MaterialAsset> materialAsset = Engine::get()->getSubSystem<Assets>()->createAsset(material, materialAssetInfo).as<MaterialAsset>();
			m_lastImportedMaterials.materials[i] = materialAsset;
			aInfo.attributes[materialID] = materialAsset.getUID();
		}
	}

	ResourceWrapper<MeshGroup> mesh = Factory<MeshGroup>::create();

	return true;
}

const ModelImporter::LastImportedMaterials& ModelImporter::getLastImportedMaterial() const
{
	return m_lastImportedMaterials;
}

void ModelImporter::processNode(const aiScene* scene, aiNode* node)
{
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		std::shared_ptr<Mesh> mesh = processMesh(scene, scene->mMeshes[node->mMeshes[i]]);

		aiMatrix4x4 transform = node->mTransformation;
		aiNode* parent = node->mParent;
		while (parent)
		{
			transform = parent->mTransformation * transform;
			parent = parent->mParent;
		}

		mesh->setRestTransform(AssimpGLMHelpers::convertMat4ToGLMFormat(transform));
		m_currentSession.mesh->addMesh(mesh);
	}

	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(scene, node->mChildren[i]);
	}
}

struct BoneWeight
{
	unsigned int boneID = -1;
	float weight = 0.f;
};

std::shared_ptr<Mesh> ModelImporter::processMesh(const aiScene* aiScene, aiMesh* aiMesh)
{
	MeshBuilder builder;

	std::shared_ptr<Mesh> generatedMesh = std::make_shared<Mesh>();

	generatedMesh->setName(aiMesh->mName.C_Str());

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
		auto& boneNameToIDMap = m_currentSession.boneNameToIDMap;
		auto& boneCount = m_currentSession.boneCount;

		// Iterate all bones in Assimp model
		for (int i = 0; i < aiMesh->mNumBones; i++)
		{
			auto bone = aiMesh->mBones[i];
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

		m_currentSession.mesh->addBonesInfo(bonesOffsets, boneNameToIDMap);
	}

	builder.setMaterialIndex(aiMesh->mMaterialIndex);

	builder.addPositions(positions)
		.addNormals(normals)
		.addTexcoords(texcoords)
		.addIndices(indices)
		.addTangents(tangents);

	// build mesh
	builder.build(*generatedMesh.get());

	return generatedMesh;
}



AssetHandle<TextureAsset> ModelImporter::copyAiMaterialTexture(const aiScene* scene,
	aiMaterial* mat, 
	aiTextureType type, 
	const std::string& dir, 
	std::unordered_map<std::string, 
	AssetHandle<TextureAsset>>& cachedTextures,
	const AssetRecord& aInfo)
{
	aiString str;
	if (mat->GetTexture(type, 0, &str) != aiReturn_SUCCESS)
	{
		return AssetHandle<TextureAsset>::empty;
	}

	ResourceWrapper<Texture> texture;
	AssetHandle<TextureAsset> AssetTexture;
	const aiTexture* aiTexture = scene->GetEmbeddedTexture(str.C_Str());
	if(aiTexture)
	{
		std::string textureName = std::filesystem::path(aiTexture->mFilename.C_Str()).filename().stem().string();

		if (!textureName.empty())
		{
			if (cachedTextures.find(textureName) != cachedTextures.end())
			{
				// Already loaded
				return cachedTextures[textureName];
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
		if (type == aiTextureType_DIFFUSE || type == aiTextureType_DIFFUSE_ROUGHNESS || type == aiTextureType_METALNESS || type == aiTextureType_AMBIENT_OCCLUSION) // todo fix
		{
			usage = TextureSemantic::Color;
		}
		else if (type == aiTextureType_NORMALS)
		{
			usage = TextureSemantic::Normal;
		}

		texture = Texture::createTexture(width,
			height,
			channels,
			Texture::getInternalFormatFromUsage(usage),
			Texture::getFormatFromChannels(channels),
			TextureType::UNSIGNED_BYTE,
			TextureFilter::Linear,
			TextureWrap::Repeat,
			pixelData);

		AssetCreateDescriptor textureAssetDesc;
		textureAssetDesc.aType = AssetType::TEXTURE;
		textureAssetDesc.name = textureName;
		textureAssetDesc.isEngineOwned = aInfo.isEngineOwned;
		textureAssetDesc.assetDirectory = aInfo.assetDirectory;
		textureAssetDesc.targetDirectory = aInfo.targetDirectory;
		AssetTexture = Engine::get()->getSubSystem<Assets>()->createAsset(texture, textureAssetDesc).as<TextureAsset>();

		if (!textureName.empty())
		{
			cachedTextures.insert({ textureName, AssetTexture });
		}
	}	
	else
	{
		std::string path = findTexture(str, dir);
		if (path.empty())
		{
			return AssetHandle<TextureAsset>::empty;
		}

		if (cachedTextures.find(path) != cachedTextures.end())
		{
			// Already loaded
			return cachedTextures[path];
		}

		Texture::TextureAssetDescriptor tSettings;
		tSettings.assetDirectory = aInfo.assetDirectory;
		tSettings.isEngineOwned = aInfo.isEngineOwned;

		tSettings.usage = TextureSemantic::Color; // todo fix
		AssetTexture = TextureAsset::import(path, tSettings);

		cachedTextures.insert({ path, AssetTexture });
	}

	return AssetTexture;
}