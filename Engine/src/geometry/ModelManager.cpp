#include "ModelManager.h"

#include "geometry/Model.h"
#include "geometry/ModelImporter.h"
#include "geometry/MeshExporter.h"
#include "geometry/MeshBinaryLoader.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

Ref<Asset> ModelTypeManager::createAsset(const AssetBuildDescriptor& assetDesc, const ResourceBuildDescriptor& resourceDesc)
{
	return createRef< ModelAsset>();
}

Ref<Asset> ModelTypeManager::deserializeAsset(const nlohmann::json& j)
{
	auto asset = createRef<ModelAsset>();
	asset->deserialize(j);
	return asset;
}

bool ModelTypeManager::importAsset(const std::string& src, ImportNode& result)
{
	ModelImporter::ModelInfo modelInfo;
	Engine::get()->getSubSystem<ModelImporter>()->parseModel(src, modelInfo);

	// Build root Model node
	//std::filesystem::path path(src);
	//result.name = path.filename().stem().string();
	//result.createDescriptor.aType = AssetType::MODEL;
	//result.createDescriptor.sourcePath = src;
	//result.createDescriptor.makeResourceCreateDescriptor<ModelCreateDescriptor>()->data = modelInfo.meshDataList;

	// Build MeshArray node
	std::filesystem::path path(src);
	result.name = path.filename().stem().string();
	result.assetDesc.aType = AssetType::MESH;
	auto rootModelDesc = result.emplaceCreateDesc<ModelCreateDescriptor>();
	rootModelDesc->data = modelInfo.meshDataList;

	// Build texture dependency nodes (embedded textures)
	//std::map<std::string, std::string> textureNameToSlot; // texture name -> slot name
	for (const TextureData& tData : modelInfo.textureDataList)
	{
		ImportNode textureNode;
		textureNode.name = tData.textureName;
		textureNode.assetDesc.aType = AssetType::TEXTURE;
		textureNode.assetDesc.name = tData.textureName;
		auto textureResourceDesc = textureNode.emplaceCreateDesc<TextureCreateDescriptor>();
		textureResourceDesc->textureData = tData;
		
		// Store texture node - we'll add it to material dependencies
		//textureNameToSlot[tData.textureName] = ""; // Will be set when mapping to materials
		result.dependencies["TEXTURE_" + tData.textureName] = textureNode;
	}

	// Build texture dependency nodes (external textures)
	for (const std::string& texturePath : modelInfo.textureFilepathList)
	{
		ImportNode textureNode;
		std::filesystem::path texPath(texturePath);
		textureNode.name = texPath.filename().stem().string();
		textureNode.assetDesc.aType = AssetType::TEXTURE;
		textureNode.assetDesc.sourcePath = texturePath;
		auto texLoadDesc = textureNode.emplaceLoadDesc<TextureLoadDescriptor>();
		texLoadDesc->sourcePath = texturePath;
		
		result.dependencies["TEXTURE_" + texturePath] = textureNode;
	}

	// Build material dependency nodes with texture dependencies
	int matIndex = 0;
	for (const MaterialData& mData : modelInfo.materialDataList)
	{
		ImportNode materialNode;
		materialNode.name = mData.name;
		materialNode.assetDesc.aType = AssetType::MATERIAL;
		materialNode.assetDesc.name = mData.name;
		auto materialResourceDesc = materialNode.emplaceCreateDesc<MaterialCreateDescriptor>();
		materialResourceDesc->data = mData;

		// Add texture dependencies to this material
		auto materialTextureIt = modelInfo.materialToTextureMap.find(mData.name);
		if (materialTextureIt != modelInfo.materialToTextureMap.end())
		{
			for (const auto& [materialTextureType, textureName] : materialTextureIt->second)
			{
				// Find the texture node in root dependencies
				std::string textureKey = "TEXTURE_" + textureName;
				auto textureIt = result.dependencies.find(textureKey);
				if (textureIt != result.dependencies.end())
				{
					// Add texture as dependency to material with slot name
					materialNode.dependencies[materialTextureType] = textureIt->second;
				}
			}
		}

		std::string slotStr = std::to_string(matIndex);

		result.dependencies["MATERIAL_" + mData.name + "_SLOT_" + slotStr] = materialNode;
		matIndex++;
	}

	return true;
}

bool ModelTypeManager::saveResource(const ResourceBuildDescriptor& desc, const ScopedPath& dst)
{
	auto meshGroupDesc = dynamic_cast<const ModelCreateDescriptor*>(&desc);
	if (!meshGroupDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	const std::vector<MeshData>& data = meshGroupDesc->data;

	//MeshExporter::exportMeshes(std::vector<MeshData>{ data }, dst.absolute().string());
	MeshBinaryLoader::save(data, dst.absolute().string());

	return true;
}

ResourceLoadDescriptor* ModelTypeManager::makeResourceLoadDescriptor()
{
	return new ModelLoadDescriptor();
}

ResourceWrapper<Resource> ModelTypeManager::loadResourceFromDisk(ResourceLoadDescriptor& desc)
{
	return ResourceWrapper<Resource>();
}

void ModelTypeManager::parse(ResourceLoadDescriptor& desc)
{
}

void ModelTypeManager::parse(ResourceBuildDescriptor& desc)
{
}