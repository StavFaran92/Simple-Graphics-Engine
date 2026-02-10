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
	Engine::get()->getSubSystem<Assets>()->importAsset(AssetType::TEXTURE, path, aDesc);
}

void BuiltInAssetsLoader::loadTextures()
{
	{
		static unsigned char* whiteColor = new unsigned char[3] { 255, 255, 255 }; // todo rethink this
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
		ResourceWrapper<Texture> textureResource = Texture::createTexture(tData);

		AssetCreateDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = "SGE_TEXTURE_WHITE";
		desc.isEngineOwned = true;
		Engine::get()->getSubSystem<Assets>()->promoteToAsset(textureResource, desc);
	}

	{
		static unsigned char* blackColor = new unsigned char[3] { 0, 0, 0};
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
		ResourceWrapper<Texture> textureResource = Texture::createTexture(tData);

		AssetCreateDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = "SGE_TEXTURE_BLACK";
		desc.isEngineOwned = true;
		Engine::get()->getSubSystem<Assets>()->promoteToAsset(textureResource, desc);
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
		auto materialResource = Material::create(MaterialRenderMode::Opaque);
		Engine::get()->getSubSystem<Assets>()->promoteToAsset(materialResource, aDesc);
	}

	{
		AssetCreateDescriptor aDesc;
		aDesc.isEngineOwned = true;
		aDesc.name = SGE_MATERIAL_TERRAIN_DEFAULT;
		auto materialResource = Material::create(MaterialRenderMode::Terrain);
		Engine::get()->getSubSystem<Assets>()->promoteToAsset(materialResource, aDesc);
	}
}

void BuiltInAssetsLoader::loadMeshes()
{
	{
		// Create box
		ResourceWrapper<Model> meshCollection = Factory<Model>::create();
		Box::createMesh(meshCollection);

		AssetCreateDescriptor aInfo;
		aInfo.name = "SGE_MESH_BOX";
		aInfo.aType = AssetType::MESH;
		aInfo.isEngineOwned = true;
		Engine::get()->getSubSystem<Assets>()->promoteToAsset(meshCollection, aInfo);
	}

	{
		ResourceWrapper<Model> meshCollection = Factory<Model>::create();
		Quad::createMesh(meshCollection);

		AssetCreateDescriptor aInfo;
		aInfo.name = "SGE_MESH_QUAD";
		aInfo.aType = AssetType::MESH;
		aInfo.isEngineOwned = true;
		Engine::get()->getSubSystem<Assets>()->promoteToAsset(meshCollection, aInfo);
	}

	{
		ResourceWrapper<Model> meshCollection = Factory<Model>::create();
		Sphere::createMesh(meshCollection, 1, 36, 36);

		AssetCreateDescriptor aInfo;
		aInfo.name = "SGE_MESH_SPHERE";
		aInfo.aType = AssetType::MESH;
		aInfo.isEngineOwned = true;
		Engine::get()->getSubSystem<Assets>()->promoteToAsset(meshCollection, aInfo);
	}

	{
		ResourceWrapper<Model> meshCollection = Factory<Model>::create();
		Grid::generateGrid(meshCollection, 10, 10);

		AssetCreateDescriptor aInfo;
		aInfo.name = "SGE_MESH_GRID";
		aInfo.aType = AssetType::MESH;
		aInfo.isEngineOwned = true;
		Engine::get()->getSubSystem<Assets>()->promoteToAsset(meshCollection, aInfo);
	}

	{
		AssetCreateDescriptor aInfo;
		aInfo.aType = AssetType::MESH;
		aInfo.name = "SGE_MESH_CAMERA";
		aInfo.isEngineOwned = true;
		Engine::get()->getSubSystem<Assets>()->importAsset(AssetType::MODEL, SGE_ROOT_DIR "Resources/Engine/Meshes/camera_v2.dae", aInfo);
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

