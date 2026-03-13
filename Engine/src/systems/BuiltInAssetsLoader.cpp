#include "systems/BuiltInAssetsLoader.h"

#include "memory/Assets.h"
#include "runtime/Context.h"
#include "serialize/ProjectAssetRegistry.h"
#include "core/CacheSystem.h"
#include "core/Factory.h"
#include <gl/glew.h>

#include "render/Material.h"
#include "texture/Texture.h"
#include "geometry/Quad.h"
#include "geometry/Box.h"
#include "geometry/Sphere.h"
#include "geometry/Grid.h"

#include "geometry/ModelImporter.h"
#include "memory/BuiltInAssets.h"

void BuiltInAssetsLoader::loadTextures()
{
	{
		static unsigned char whiteColor[3] = { 255, 255, 255 };

		AssetCreateDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = "SGE_TEXTURE_WHITE";
		desc.isEngineOwned = true;
		auto createDesc = desc.makeResourceCreateDescriptor<TextureCreateDescriptor>();
		createDesc->textureData.target = TextureTarget::TEXTURE_2D;
		createDesc->textureData.width = 1;
		createDesc->textureData.height = 1;
		createDesc->textureData.channels = 3;
		createDesc->textureData.data = whiteColor;
		createDesc->textureData.internalFormat = TextureInternalFormat::RGB;
		createDesc->textureData.format = TextureFormat::RGB;
		createDesc->textureData.type = TextureType::UNSIGNED_BYTE;
		createDesc->textureData.filter = TextureFilter::Linear;
		createDesc->textureData.wrap = TextureWrap::Repeat;
		createDesc->textureData.textureName = "SGE_TEXTURE_WHITE";
		Engine::get()->getSubSystem<Assets>()->createAsset(desc);
	}

	{
		static unsigned char blackColor[3] = { 0, 0, 0 };

		AssetCreateDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = "SGE_TEXTURE_BLACK";
		desc.isEngineOwned = true;
		auto createDesc = desc.makeResourceCreateDescriptor<TextureCreateDescriptor>();
		createDesc->textureData.target = TextureTarget::TEXTURE_2D;
		createDesc->textureData.width = 1;
		createDesc->textureData.height = 1;
		createDesc->textureData.channels = 3;
		createDesc->textureData.data = blackColor;
		createDesc->textureData.internalFormat = TextureInternalFormat::RGB;
		createDesc->textureData.format = TextureFormat::RGB;
		createDesc->textureData.type = TextureType::UNSIGNED_BYTE;
		createDesc->textureData.filter = TextureFilter::Linear;
		createDesc->textureData.wrap = TextureWrap::Repeat;
		createDesc->textureData.textureName = "SGE_TEXTURE_BLACK";
		Engine::get()->getSubSystem<Assets>()->createAsset(desc);
	}

	{
		AssetCreateDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.isEngineOwned = true;
		desc.name = "SGE_TEXTURE_GRASS";
		desc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Textures/Ground037_1K-JPG_Color.jpg";
		Engine::get()->getSubSystem<Assets>()->importAsset(desc);
	}
}

void BuiltInAssetsLoader::loadMaterials()
{
	// TODO: procedural material creation needs MaterialCreateDescriptor + IResourceFactory impl
	{
		AssetCreateDescriptor desc;
		desc.aType = AssetType::MATERIAL;
		desc.isEngineOwned = true;
		desc.name = SGE_MATERIAL_DEFAULT;
		auto createDesc = desc.makeResourceCreateDescriptor<MaterialCreateDescriptor>();
		createDesc->data.renderMode = MaterialRenderMode::Opaque;
		Engine::get()->getSubSystem<Assets>()->createAsset(desc);
	}

	{
		AssetCreateDescriptor desc;
		desc.aType = AssetType::MATERIAL;
		desc.isEngineOwned = true;
		desc.name = SGE_MATERIAL_TERRAIN_DEFAULT;
		auto createDesc = desc.makeResourceCreateDescriptor<MaterialCreateDescriptor>();
		createDesc->data.renderMode = MaterialRenderMode::Terrain;
		Engine::get()->getSubSystem<Assets>()->createAsset(desc);
	}
}

void BuiltInAssetsLoader::loadMeshes()
{
	//TODO
	// the use of ::createMesh can and should be optimized, instead of building the mesh
	// binding VBOs all on the GPU, it should return the raw data directly on the CPU.

	{
		AssetCreateDescriptor desc;
		desc.aType = AssetType::MESH;
		desc.name = "SGE_MESH_BOX";
		desc.isEngineOwned = true;
		auto meshDesc = desc.makeResourceCreateDescriptor<ModelCreateDescriptor>();
		meshDesc->data = std::vector<MeshData>{ Box::createMesh()->getMeshData() };

		Engine::get()->getSubSystem<Assets>()->createAsset(desc);
	}

	{
		AssetCreateDescriptor desc;
		desc.aType = AssetType::MESH;
		desc.name = "SGE_MESH_QUAD";
		desc.isEngineOwned = true;
		auto meshDesc = desc.makeResourceCreateDescriptor<ModelCreateDescriptor>();
		meshDesc->data = std::vector<MeshData>{ Quad::createMesh()->getMeshData() };
		Engine::get()->getSubSystem<Assets>()->createAsset(desc);
	}

	{
		AssetCreateDescriptor desc;
		desc.aType = AssetType::MESH;
		desc.name = "SGE_MESH_SPHERE";
		desc.isEngineOwned = true;
		auto meshDesc = desc.makeResourceCreateDescriptor<ModelCreateDescriptor>();
		meshDesc->data = std::vector<MeshData>{ Sphere::createMesh(1, 36, 36)->getMeshData() };
		Engine::get()->getSubSystem<Assets>()->createAsset(desc);
	}

	{
		AssetCreateDescriptor desc;
		desc.aType = AssetType::MESH;
		desc.name = "SGE_MESH_GRID";
		desc.isEngineOwned = true;
		auto meshDesc = desc.makeResourceCreateDescriptor<ModelCreateDescriptor>();
		meshDesc->data = std::vector<MeshData>{ Grid::createMesh(10, 10)->getMeshData() };
		Engine::get()->getSubSystem<Assets>()->createAsset(desc);
	}

	{
		AssetCreateDescriptor desc;
		desc.aType = AssetType::MESH;
		desc.name = "SGE_MESH_CAMERA";
		desc.isEngineOwned = true;
		desc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Meshes/camera_v2.dae";
		desc.makeResourceLoadDescriptor<ModelLoadDescriptor>();
		Engine::get()->getSubSystem<Assets>()->importAsset(desc);
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
