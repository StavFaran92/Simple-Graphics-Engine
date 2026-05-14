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
#include "geometry/Cylinder.h"
#include "geometry/Sphere.h"
#include "geometry/Grid.h"

#include "geometry/ModelImporter.h"
#include "memory/BuiltInAssets.h"

static Assets* assets() { return Engine::get()->getSubSystem<Assets>(); }

void BuiltInAssetsLoader::loadTextures()
{
	if (!assets()->hasAsset(SGE_TEXTURE_WHITE))
	{
		static unsigned char whiteColor[3] = { 255, 255, 255 };

		AssetBuildDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = SGE_TEXTURE_WHITE;
		desc.isEngineOwned = true;
		desc.visibility = AssetVisibility::Public;
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
		createDesc.textureData.textureName = SGE_TEXTURE_WHITE;
		assets()->createAsset(desc, createDesc);
	}

	if (!assets()->hasAsset(SGE_TEXTURE_BLACK))
	{
		static unsigned char blackColor[3] = { 0, 0, 0 };

		AssetBuildDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.name = SGE_TEXTURE_BLACK;
		desc.isEngineOwned = true;
		desc.visibility = AssetVisibility::Public;
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
		createDesc.textureData.textureName = SGE_TEXTURE_BLACK;
		assets()->createAsset(desc, createDesc);
	}

	if (!assets()->hasAsset(SGE_TEXTURE_GRASS))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.isEngineOwned = true;
		desc.visibility = AssetVisibility::Public;
		desc.name = SGE_TEXTURE_GRASS;
		TextureLoadDescriptor loadDesc;
		loadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Textures/Ground037_1K-JPG_Color.jpg";
		assets()->importAsset(desc, loadDesc);
	}

	if (!assets()->hasAsset(SGE_TEXTURE_CHECKERBOARD))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.isEngineOwned = true;
		desc.name = SGE_TEXTURE_CHECKERBOARD;
		TextureLoadDescriptor loadDesc;
		loadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Textures/Checkerboard_pattern.png";
		assets()->importAsset(desc, loadDesc);
	}

	if (!assets()->hasAsset(SGE_TEXTURE_TERRAIN_CHECKERBOARD))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.isEngineOwned = true;
		desc.visibility = AssetVisibility::Public;
		desc.name = SGE_TEXTURE_TERRAIN_CHECKERBOARD;
		TextureLoadDescriptor loadDesc;
		loadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Textures/checkerboard.jpg";
		assets()->importAsset(desc, loadDesc);
	}

	if (!assets()->hasAsset(SGE_TEXTURE_TILE_NOISE))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::TEXTURE;
		desc.isEngineOwned = true;
		desc.name = SGE_TEXTURE_TILE_NOISE;
		TextureLoadDescriptor loadDesc;
		loadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Textures/texTileNoise.png";
		assets()->importAsset(desc, loadDesc);
	}
}

void BuiltInAssetsLoader::loadMaterials()
{
	if (!assets()->hasAsset(SGE_MATERIAL_DEFAULT))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MATERIAL;
		desc.isEngineOwned = true;
		desc.name = SGE_MATERIAL_DEFAULT;
		desc.visibility = AssetVisibility::Public;
		MaterialCreateDescriptor createDesc;
		createDesc.data.setMaterialRenderMode(MaterialRenderMode::Opaque);
		assets()->createAsset(desc, createDesc);
	}

	if (!assets()->hasAsset(SGE_MATERIAL_TERRAIN_DEFAULT))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MATERIAL;
		desc.isEngineOwned = true;
		desc.name = SGE_MATERIAL_TERRAIN_DEFAULT;
		desc.visibility = AssetVisibility::Public;
		MaterialCreateDescriptor createDesc;
		createDesc.data.setMaterialRenderMode(MaterialRenderMode::Terrain);
		auto materialAsset = assets()->createAsset(desc, createDesc).as<MaterialAsset>();

		auto sampler = std::make_shared<TextureSamplerAsset>();
		sampler->texture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_TERRAIN_CHECKERBOARD);
		sampler->state.isActive = true;
		materialAsset->setProperty("samplerAlbedo", sampler);
	}
}

void BuiltInAssetsLoader::loadMeshes()
{
	if (!assets()->hasAsset(SGE_MESH_BOX))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = SGE_MESH_BOX;
		desc.isEngineOwned = true;
		desc.visibility = AssetVisibility::Public;
		ModelCreateDescriptor meshDesc;
		meshDesc.data.m_meshes = std::vector<MeshData>{ Box::createMesh()->getMeshData() };
		assets()->createAsset(desc, meshDesc);
	}

	if (!assets()->hasAsset(SGE_MESH_QUAD))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = SGE_MESH_QUAD;
		desc.isEngineOwned = true;
		desc.visibility = AssetVisibility::Public;
		ModelCreateDescriptor meshDesc;
		meshDesc.data.m_meshes = std::vector<MeshData>{ Quad::createMesh()->getMeshData() };
		assets()->createAsset(desc, meshDesc);
	}

	if (!assets()->hasAsset(SGE_MESH_SPHERE))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = SGE_MESH_SPHERE;
		desc.isEngineOwned = true;
		desc.visibility = AssetVisibility::Public;
		ModelCreateDescriptor meshDesc;
		meshDesc.data.m_meshes = std::vector<MeshData>{ Sphere::createMesh(1, 36, 36)->getMeshData() };
		assets()->createAsset(desc, meshDesc);
	}

	if (!assets()->hasAsset(SGE_MESH_CYLINDER))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = SGE_MESH_CYLINDER;
		desc.isEngineOwned = true;
		desc.visibility = AssetVisibility::Public;
		ModelCreateDescriptor meshDesc;
		meshDesc.data.m_meshes = std::vector<MeshData>{ Cylinder::createMesh(1, 1, 36)->getMeshData() };
		assets()->createAsset(desc, meshDesc);
	}

	if (!assets()->hasAsset(SGE_MESH_GRID))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = SGE_MESH_GRID;
		desc.isEngineOwned = true;
		ModelCreateDescriptor meshDesc;
		meshDesc.data.m_meshes = std::vector<MeshData>{ Grid::createMesh(10, 10)->getMeshData() };
		assets()->createAsset(desc, meshDesc);
	}

	if (!assets()->hasAsset(SGE_MESH_CAMERA))
	{
		AssetBuildDescriptor desc;
		desc.aType = AssetType::MODEL;
		desc.name = SGE_MESH_CAMERA;
		desc.isEngineOwned = true;
		ModelLoadDescriptor loadDesc;
		loadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Meshes/camera_v2.dae";
		assets()->importAsset(desc, loadDesc);
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
