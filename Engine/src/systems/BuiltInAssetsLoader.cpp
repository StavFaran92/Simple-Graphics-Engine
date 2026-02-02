#include "systems/BuiltInAssetsLoader.h"

#include "memory/Assets.h"
#include "runtime/Context.h"
#include "serialize/ProjectAssetRegistry.h"
#include "core/CacheSystem.h"
#include "core/Factory.h"
#include <gl/glew.h>

#include "render/Material.h"
#include "geometry/Quad.h"
#include "geometry/Box.h"
#include "geometry/Sphere.h"
#include "geometry/Grid.h"

#include "geometry/ModelImporter.h"
#include "memory/BuiltInAssets.h"


void acquireTexture(const std::string& name, const std::string& path)
{
	AssetCreateDescriptor aDesc;
	aDesc.isEngineOwned = true;
	aDesc.name = name;
	TextureAsset::import(path, aDesc);
}

void BuiltInAssetsLoader::loadTextures()
{
	{
		static unsigned char* whiteColor = new unsigned char[3] { 255, 255, 255 }; // todo rethink this
		AssetCreateDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = "SGE_TEXTURE_WHITE";
		desc.isEngineOwned = true;
		desc.createFunc = []() ->ResourceWrapper<Resource> {
			TextureData tData;
			tData.target = TextureTarget::TEXTURE_2D;
			tData.width = 1;
			tData.height = 1;
			tData.channels = 3;
			tData.data = whiteColor;
			tData.internalFormat = TextureInternalFormat::RGB;
			tData.format = TextureFormat::RGB;
			tData.type = TextureType::UNSIGNED_BYTE;
			tData.filter = TextureFilter::Linear;
			tData.wrap = TextureWrap::Repeat;
			return Texture::createTexture(tData);
		};
		TextureAsset::create(desc);
	}

	{
		static unsigned char* blackColor = new unsigned char[3] { 0, 0, 0};
		AssetCreateDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = "SGE_TEXTURE_BLACK";
		desc.isEngineOwned = true;
		desc.createFunc = []() ->ResourceWrapper<Resource> {
			TextureData tData;
			tData.target = TextureTarget::TEXTURE_2D;
			tData.width = 1;
			tData.height = 1;
			tData.channels = 3;
			tData.data = blackColor;
			tData.internalFormat = TextureInternalFormat::RGB;
			tData.format = TextureFormat::RGB;
			tData.type = TextureType::UNSIGNED_BYTE;
			tData.filter = TextureFilter::Linear;
			tData.wrap = TextureWrap::Repeat;
			return Texture::createTexture(tData);
		};
		TextureAsset::create(desc);
	}

	acquireTexture("SGE_TEXTURE_GRASS", SGE_ROOT_DIR "Resources/Engine/Textures/Ground037_1K-JPG_Color.jpg");
}

void BuiltInAssetsLoader::loadMaterials()
{
	{
		AssetCreateDescriptor aDesc;
		aDesc.isEngineOwned = true;
		aDesc.name = SGE_MATERIAL_DEFAULT;
		aDesc.aType = AssetType::MATERIAL;
		aDesc.createFunc = []() ->ResourceWrapper<Resource> {
			return Material::create(MaterialRenderMode::Opaque);
		};
		
		MaterialAsset::create(aDesc);
	}

	{
		AssetCreateDescriptor aDesc;
		aDesc.isEngineOwned = true;
		aDesc.name = SGE_MATERIAL_TERRAIN_DEFAULT;
		aDesc.createFunc = []() ->ResourceWrapper<Resource> {
			return Material::create(MaterialRenderMode::Terrain);
			};
		MaterialAsset::create(aDesc);
	}
}

void BuiltInAssetsLoader::loadMeshes()
{
	{
		// Create box

		AssetCreateDescriptor aInfo;
		aInfo.name = "SGE_MESH_BOX";
		aInfo.isEngineOwned = true;
		aInfo.createFunc = []() ->ResourceWrapper<Resource> {
			ResourceWrapper<MeshGroup> meshCollection = Factory<MeshGroup>::create();
			Box::createMesh(meshCollection);
			return meshCollection;
			};
		MeshGroupAsset::create(aInfo);
	}

	{
		AssetCreateDescriptor aInfo;
		aInfo.name = "SGE_MESH_QUAD";
		aInfo.isEngineOwned = true;
		aInfo.createFunc = []() -> ResourceWrapper<Resource> {
			ResourceWrapper<MeshGroup> meshCollection = Factory<MeshGroup>::create();
			Quad::createMesh(meshCollection);
			return meshCollection;
			};

		MeshGroupAsset::create(aInfo);
	}

	{
		AssetCreateDescriptor aInfo;
		aInfo.name = "SGE_MESH_SPHERE";
		aInfo.isEngineOwned = true;
		aInfo.createFunc = []() -> ResourceWrapper<Resource> {
			ResourceWrapper<MeshGroup> meshCollection = Factory<MeshGroup>::create();
			Sphere::createMesh(meshCollection, 1, 36, 36);
			return meshCollection;
			};

		MeshGroupAsset::create(aInfo);
	}

	{
		AssetCreateDescriptor aInfo;
		aInfo.name = "SGE_MESH_GRID";
		aInfo.isEngineOwned = true;
		aInfo.createFunc = []() -> ResourceWrapper<Resource> {
			ResourceWrapper<MeshGroup> meshCollection = Factory<MeshGroup>::create();
			Grid::generateGrid(meshCollection, 10, 10);
			return meshCollection;
			};

		MeshGroupAsset::create(aInfo);
	}

	{
		AssetCreateDescriptor aInfo;
		aInfo.aType = AssetType::MESH;
		aInfo.name = "SGE_MESH_CAMERA";
		aInfo.isEngineOwned = true;
		MeshGroupLoadDescriptor* resourceDesc = aInfo.makeResourceDescriptor<MeshGroupLoadDescriptor>();
		MeshGroupAsset::import(SGE_ROOT_DIR "Resources/Engine/Meshes/camera_v2.dae", aInfo);
	}
}

void BuiltInAssetsLoader::loadShaders()
{
}

void BuiltInAssetsLoader::loadAssets()
{
	loadShaders();
	loadTextures();
	loadMaterials();
	loadMeshes();
}

