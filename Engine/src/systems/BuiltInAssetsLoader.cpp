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

void addAsAsset(const std::string& name, const ResourceWrapper<MeshGroup>& meshCollection)
{
	AssetCreateDescriptor aInfo;
	aInfo.name = name;
	aInfo.isEngineOwned = true;
	MeshGroupAsset::create(meshCollection, aInfo);
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
		tData.textureName = "SGE_TEXTURE_WHITE";

		auto texture = Texture::createTexture(tData);

		AssetCreateDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = tData.textureName;
		desc.isEngineOwned = true;
		TextureAsset::create(texture, desc);
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
		tData.textureName = "SGE_TEXTURE_BLACK";
		auto texture = Texture::createTexture(tData);

		AssetCreateDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = tData.textureName;
		desc.isEngineOwned = true;
		TextureAsset::create(texture, desc);
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
		ResourceWrapper<Material> material = Material::create(MaterialRenderMode::Opaque);
		MaterialAsset::create(material, aDesc);
	}

	{
		AssetCreateDescriptor aDesc;
		aDesc.isEngineOwned = true;
		aDesc.name = SGE_MATERIAL_TERRAIN_DEFAULT;
		ResourceWrapper<Material> material = Material::create(MaterialRenderMode::Terrain);
		MaterialAsset::create(material, aDesc);
	}
}

void BuiltInAssetsLoader::loadMeshes()
{
	{
		// Create box
		ResourceWrapper<MeshGroup> meshCollection = Factory<MeshGroup>::create();
		Box::createMesh(meshCollection);
		addAsAsset("SGE_MESH_BOX", meshCollection);
	}

	{
		// Create Quad
		ResourceWrapper<MeshGroup> meshCollection = Factory<MeshGroup>::create();
		Quad::createMesh(meshCollection);
		addAsAsset("SGE_MESH_QUAD", meshCollection);
	}

	{
		// Create sphere
		ResourceWrapper<MeshGroup> meshCollection = Factory<MeshGroup>::create();
		Sphere::createMesh(meshCollection, 1, 36, 36);
		addAsAsset("SGE_MESH_SPHERE", meshCollection);
	}

	{
		ResourceWrapper<MeshGroup> meshCollection = Factory<MeshGroup>::create();
		Grid::generateGrid(meshCollection, 10, 10);
		addAsAsset("SGE_MESH_GRID", meshCollection);
	}

	{
		AssetCreateDescriptor aInfo;
		aInfo.aType = AssetType::MESH;
		aInfo.name = "SGE_MESH_CAMERA";
		aInfo.isEngineOwned = true;
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

