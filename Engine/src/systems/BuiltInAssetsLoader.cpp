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

		AssetBuildDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = "SGE_TEXTURE_WHITE";
		desc.isEngineOwned = true;
		TextureCreateDescriptor createDesc;
		createDesc.textureData.target = TextureTarget::TEXTURE_2D;
		createDesc.textureData.width = 1;
		createDesc.textureData.height = 1;
		createDesc.textureData.channels = 3;
		createDesc.textureData.data = ImageBuffer(whiteColor, 3);
		createDesc.textureData.internalFormat = TextureInternalFormat::RGB;
		createDesc.textureData.format = TextureFormat::RGB;
		createDesc.textureData.type = TextureType::UNSIGNED_BYTE;
		createDesc.textureData.filter = TextureFilter::Linear;
		createDesc.textureData.wrap = TextureWrap::Repeat;
		createDesc.textureData.textureName = "SGE_TEXTURE_WHITE";
		Engine::get()->getSubSystem<Assets>()->createAsset(desc, createDesc);
	}

	{
		static unsigned char blackColor[3] = { 0, 0, 0 };

		AssetBuildDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = "SGE_TEXTURE_BLACK";
		desc.isEngineOwned = true;
		TextureCreateDescriptor createDesc;
		createDesc.textureData.target = TextureTarget::TEXTURE_2D;
		createDesc.textureData.width = 1;
		createDesc.textureData.height = 1;
		createDesc.textureData.channels = 3;
		createDesc.textureData.data = ImageBuffer(blackColor, 3);
		createDesc.textureData.internalFormat = TextureInternalFormat::RGB;
		createDesc.textureData.format = TextureFormat::RGB;
		createDesc.textureData.type = TextureType::UNSIGNED_BYTE;
		createDesc.textureData.filter = TextureFilter::Linear;
		createDesc.textureData.wrap = TextureWrap::Repeat;
		createDesc.textureData.textureName = "SGE_TEXTURE_BLACK";
		Engine::get()->getSubSystem<Assets>()->createAsset(desc, createDesc);
	}

	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.isEngineOwned = true;
		desc.name = "SGE_TEXTURE_GRASS";
		TextureLoadDescriptor loadDesc;
		loadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Textures/Ground037_1K-JPG_Color.jpg";
		Engine::get()->getSubSystem<Assets>()->importAsset(desc, loadDesc);
	}

	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.isEngineOwned = true;
		desc.name = "SGE_TEXTURE_CHECKERBOARD";
		TextureLoadDescriptor loadDesc;
		loadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Textures/checkerboard.jpg";
		Engine::get()->getSubSystem<Assets>()->importAsset(desc, loadDesc);
	}
}

void BuiltInAssetsLoader::loadMaterials()
{
	// TODO: procedural material creation needs MaterialCreateDescriptor + IResourceFactory impl
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MATERIAL;
		desc.isEngineOwned = true;
		desc.name = SGE_MATERIAL_DEFAULT;
		MaterialCreateDescriptor createDesc;
		createDesc.data.setMaterialRenderMode(MaterialRenderMode::Opaque);
		Engine::get()->getSubSystem<Assets>()->createAsset(desc, createDesc);
	}

	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MATERIAL;
		desc.isEngineOwned = true;
		desc.name = SGE_MATERIAL_TERRAIN_DEFAULT;
		MaterialCreateDescriptor createDesc;
		createDesc.data.setMaterialRenderMode(MaterialRenderMode::Terrain);
		auto materialAsset = Engine::get()->getSubSystem<Assets>()->createAsset(desc, createDesc).as<MaterialAsset>();

		auto sampler = std::make_shared<TextureSamplerAsset>();
		sampler->texture = BuiltInAssets::getByName<TextureAsset>("SGE_TEXTURE_CHECKERBOARD");
		sampler->state.isActive = true;
		materialAsset->setProperty("samplerAlbedo", sampler);
	}
}

void BuiltInAssetsLoader::loadMeshes()
{
	//TODO
	// the use of ::createMesh can and should be optimized, instead of building the mesh
	// binding VBOs all on the GPU, it should return the raw data directly on the CPU.

	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = "SGE_MESH_BOX";
		desc.isEngineOwned = true;
		ModelCreateDescriptor meshDesc;
		meshDesc.data.m_meshes = std::vector<MeshData>{ Box::createMesh()->getMeshData() };

		Engine::get()->getSubSystem<Assets>()->createAsset(desc, meshDesc);
	}

	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = "SGE_MESH_QUAD";
		desc.isEngineOwned = true;
		ModelCreateDescriptor meshDesc;
		meshDesc.data.m_meshes = std::vector<MeshData>{ Quad::createMesh()->getMeshData() };
		Engine::get()->getSubSystem<Assets>()->createAsset(desc, meshDesc);
	}

	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = "SGE_MESH_SPHERE";
		desc.isEngineOwned = true;
		ModelCreateDescriptor meshDesc;
		meshDesc.data.m_meshes = std::vector<MeshData>{ Sphere::createMesh(1, 36, 36)->getMeshData() };
		Engine::get()->getSubSystem<Assets>()->createAsset(desc, meshDesc);
	}

	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = "SGE_MESH_GRID";
		desc.isEngineOwned = true;
		ModelCreateDescriptor meshDesc;
		meshDesc.data.m_meshes = std::vector<MeshData>{ Grid::createMesh(10, 10)->getMeshData() };
		Engine::get()->getSubSystem<Assets>()->createAsset(desc, meshDesc);
	}

	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = "SGE_MESH_CAMERA";
		desc.isEngineOwned = true;
		ModelLoadDescriptor loadDesc;
		loadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Meshes/camera_v2.dae";
		Engine::get()->getSubSystem<Assets>()->importAsset(desc, loadDesc);
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
