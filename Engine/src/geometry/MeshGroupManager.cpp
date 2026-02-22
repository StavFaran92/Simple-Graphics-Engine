#include "MeshGroupManager.h"

#include "geometry/MeshGroup.h"
#include "geometry/ModelImporter.h"
#include "geometry/MeshExporter.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

Asset* MeshGroupTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return new MeshGroupAsset(desc);
}

bool MeshGroupTypeManager::importAsset(const std::string& src, const ScopedPath& dst)
{
	// we want to do the most work we can do here, so it will not need to happen during load
	// we open the file and parse all its inner data
	// and so at the end what imported is
	// array of meshes
	// textures
	// materials
	// skeleton
	// etc
	// so i may need to alter the signature to store dependant assets
	// or at least connect them somehow.
	// so what i will do is open the file using assimp 
	// and bring mesh currently as OBJ.
	// and for each dependant assets currently bring them also, no connection ATM

	ModelImporter::ModelInfo modelInfo;
	Engine::get()->getSubSystem<ModelImporter>()->parseModel(src, modelInfo);

	// save Mesh resource
	MeshExporter::exportMeshes(modelInfo.meshDataList, dst.absolute().string());

	std::map<std::string, AssetHandle<TextureAsset>> textures;
	std::map<std::string, AssetHandle<MaterialAsset>> materials;

	// Load Embedded Textures 
	for (const TextureData& tData : modelInfo.textureDataList)
	{
		AssetCreateDescriptor textureAssetDesc;
		textureAssetDesc.aType = AssetType::TEXTURE;
		textureAssetDesc.name = tData.textureName;
		auto TextureResourceDesc = textureAssetDesc.makeResourceCreateDescriptor<TextureCreateDescriptor>();
		TextureResourceDesc->textureData = tData;
		AssetHandle<TextureAsset> textureAsset = Engine::get()->getSubSystem<Assets>()->createAsset(textureAssetDesc).as<TextureAsset>();
		textures[tData.textureName] = textureAsset;
	}

	// Load External Textures 
	for (const std::string& texturePath : modelInfo.textureFilepathList)
	{
		AssetCreateDescriptor textureAssetDesc;
		textureAssetDesc.aType = AssetType::TEXTURE;
		auto TextureResourceDesc = textureAssetDesc.makeResourceLoadDescriptor<TextureLoadDescriptor>();
		TextureResourceDesc->sourcePath = texturePath;
		AssetHandle<TextureAsset> textureAsset = Engine::get()->getSubSystem<Assets>()->importAsset(textureAssetDesc).as<TextureAsset>();
		textures[texturePath] = textureAsset;
	}

	// Load Materials
	for (const MaterialData& mData : modelInfo.materialDataList)
	{
		AssetCreateDescriptor materialAssetDesc;
		materialAssetDesc.aType = AssetType::MATERIAL;
		materialAssetDesc.name = mData.name;
		auto materialResourceDesc = materialAssetDesc.makeResourceCreateDescriptor<MaterialCreateDescriptor>();
		materialResourceDesc->data = mData;
		AssetHandle<MaterialAsset> materialAsset = Engine::get()->getSubSystem<Assets>()->createAsset(materialAssetDesc).as<MaterialAsset>();
		materials[mData.name] = materialAsset;
	}

	// Bind Textures to Materials
	for (const auto& [_, materialSpec]: modelInfo.materialToTextureMap)
	{
		for (const auto& [materialTextureType, textureName] : materialSpec)
		{

		}
	}

	// Bind Materials to Meshes


	return true;
}

bool MeshGroupTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	auto meshGroupDesc = dynamic_cast<const MeshGroupCreateDescriptor*>(&desc);
	if (!meshGroupDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	const MeshData& data = meshGroupDesc->data;

	MeshExporter::exportMeshes(std::vector<MeshData>{ data }, dst.absolute().string());

	return true;
}

ResourceLoadDescriptor* MeshGroupTypeManager::makeResourceLoadDescriptor()
{
	return new MeshGroupLoadDescriptor();
}

ResourceWrapper<Resource> MeshGroupTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void MeshGroupTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void MeshGroupTypeManager::parse(ResourceCreateDescriptor& desc)
{
}