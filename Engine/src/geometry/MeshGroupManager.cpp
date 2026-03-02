#include "MeshGroupManager.h"

#include "geometry/MeshGroup.h"
#include "geometry/ModelImporter.h"
#include "geometry/MeshExporter.h"
#include "geometry/MeshBinaryLoader.h"
#include "memory/AssetDescriptors.h"
#include "memory/AssetRecord.h"
#include "memory/AssetHandle.h"
#include "core/Engine.h"

Asset* MeshGroupTypeManager::createAsset(AssetCreateDescriptor& desc)
{
	return new MeshGroupAsset(desc);
}

bool MeshGroupTypeManager::importAsset(const std::string& src, const ScopedPath& dst, ImportNode& result)
{
	ModelImporter::ModelInfo modelInfo;
	Engine::get()->getSubSystem<ModelImporter>()->parseModel(src, modelInfo);

	// save Mesh resource
	//MeshExporter::exportMeshes(modelInfo.meshDataList, dst.absolute().string());
	MeshBinaryLoader::save(modelInfo.meshDataList, dst.absolute().string());

	// Build root MeshGroup node
	std::filesystem::path path(src);
	result.name = path.filename().stem().string();
	result.createDescriptor.aType = AssetType::MESH; // TODO fix
	result.createDescriptor.sourcePath = src;
	result.createDescriptor.makeResourceLoadDescriptor<MeshGroupLoadDescriptor>()->sourcePath = dst.absolute().string();

	// Build texture dependency nodes (embedded textures)
	//std::map<std::string, std::string> textureNameToSlot; // texture name -> slot name
	for (const TextureData& tData : modelInfo.textureDataList)
	{
		ImportNode textureNode;
		textureNode.name = tData.textureName;
		textureNode.createDescriptor.aType = AssetType::TEXTURE;
		textureNode.createDescriptor.name = tData.textureName;
		auto textureResourceDesc = textureNode.createDescriptor.makeResourceCreateDescriptor<TextureCreateDescriptor>();
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
		textureNode.createDescriptor.aType = AssetType::TEXTURE;
		textureNode.createDescriptor.sourcePath = texturePath;
		textureNode.createDescriptor.makeResourceLoadDescriptor<TextureLoadDescriptor>()->sourcePath = texturePath;
		
		result.dependencies["TEXTURE_" + texturePath] = textureNode;
	}

	// Build material dependency nodes with texture dependencies
	int matIndex = 0;
	for (const MaterialData& mData : modelInfo.materialDataList)
	{
		ImportNode materialNode;
		materialNode.name = mData.name;
		materialNode.createDescriptor.aType = AssetType::MATERIAL;
		materialNode.createDescriptor.name = mData.name;
		auto materialResourceDesc = materialNode.createDescriptor.makeResourceCreateDescriptor<MaterialCreateDescriptor>();
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

bool MeshGroupTypeManager::saveResource(const ResourceCreateDescriptor& desc, const ScopedPath& dst)
{
	auto meshGroupDesc = dynamic_cast<const MeshGroupCreateDescriptor*>(&desc);
	if (!meshGroupDesc)
	{
		logError("Invalid Descriptor specified.");
		return false;
	}

	const MeshData& data = meshGroupDesc->data;

	//MeshExporter::exportMeshes(std::vector<MeshData>{ data }, dst.absolute().string());
	MeshBinaryLoader::save(std::vector<MeshData>{ data }, dst.absolute().string());

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