#include "systems/BuiltInAssetsLoader.h"

#include "memory/Assets.h"
#include "texture/Cubemap.h"
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
	Texture::TextureAssetDescriptor aDesc;
	aDesc.isEngineOwned = true;
	aDesc.name = name;
	Texture::import(path, aDesc);
}

void addAsAsset(const std::string& name, const ResourceWrapper<MeshCollection>& meshCollection)
{
	AssetCreateDescriptor aInfo;
	aInfo.aType = AssetType::MESH;
	aInfo.name = name;
	aInfo.isEngineOwned = true;
	Engine::get()->getSubSystem<Assets>()->createAsset(meshCollection, aInfo);
}

void BuiltInAssetsLoader::loadTextures()
{
	{
		static unsigned char* whiteColor = new unsigned char[3] { 255, 255, 255 }; // todo rethink this

		Texture::TextureData tData;
		tData.target = Texture::TextureTarget::TEXTURE_2D;
		tData.width = 1;
		tData.height = 1;
		tData.bpp = 3;
		tData.data = whiteColor;
		tData.internalFormat = Texture::InternalFormat::RGB2;
		tData.format = Texture::Format::RGB;
		tData.type = Texture::Type::UNSIGNED_BYTE;
		tData.isEngineOwned = true;
		tData.textureName = "SGE_TEXTURE_WHITE";
		tData.params = { {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
						{GL_TEXTURE_MAG_FILTER, GL_LINEAR},
						{GL_TEXTURE_WRAP_S, GL_REPEAT},
						{GL_TEXTURE_WRAP_T, GL_REPEAT } };

		auto texture = Texture::create2DTextureFromBuffer(tData);

		AssetCreateDescriptor aInfo;
		aInfo.aType = AssetType::TEXTURE;
		aInfo.name = tData.textureName;
		aInfo.isEngineOwned = true;
		aInfo.attributes = texture->getTextureAssetAttributes().toMap();
		Engine::get()->getSubSystem<Assets>()->createAsset(texture, aInfo);
	}

	{
		static unsigned char* blackColor = new unsigned char[3] { 0, 0, 0};
		Texture::TextureData tData;
		tData.target = Texture::TextureTarget::TEXTURE_2D;
		tData.width = 1;
		tData.height = 1;
		tData.bpp = 3;
		tData.data = blackColor;
		tData.internalFormat = Texture::InternalFormat::RGB2;
		tData.format = Texture::Format::RGB;
		tData.type = Texture::Type::UNSIGNED_BYTE;
		tData.isEngineOwned = true;
		tData.textureName = "SGE_TEXTURE_BLACK";
		tData.params = { {GL_TEXTURE_MIN_FILTER, GL_LINEAR},
						{GL_TEXTURE_MAG_FILTER, GL_LINEAR},
						{GL_TEXTURE_WRAP_S, GL_REPEAT},
						{GL_TEXTURE_WRAP_T, GL_REPEAT } };
		auto texture = Texture::create2DTextureFromBuffer(tData);

		AssetCreateDescriptor aInfo;
		aInfo.aType = AssetType::TEXTURE;
		aInfo.name = tData.textureName;
		aInfo.isEngineOwned = true;
		aInfo.attributes = texture->getTextureAssetAttributes().toMap();
		Engine::get()->getSubSystem<Assets>()->createAsset(texture, aInfo);
	}

	acquireTexture("SGE_TEXTURE_GRASS", SGE_ROOT_DIR + "Resources/Engine/Textures/Ground037_1K-JPG_Color.jpg");
}

void BuiltInAssetsLoader::loadMaterials()
{
	{
		AssetCreateDescriptor aDesc;
		aDesc.isEngineOwned = true;
		aDesc.name = "SGE_MATERIAL_DEFAULT";
		aDesc.aType = AssetType::MATERIAL;
		ResourceWrapper<Material> material = Material::create();
		Engine::get()->getSubSystem<Assets>()->createAsset(material, aDesc);
	}
}

void BuiltInAssetsLoader::loadMeshes()
{
	{
		// Create box
		ResourceWrapper<MeshCollection> meshCollection = Factory<MeshCollection>::create();
		Box::createMesh(meshCollection);
		addAsAsset("SGE_MESH_BOX", meshCollection);
	}

	{
		// Create Quad
		ResourceWrapper<MeshCollection> meshCollection = Factory<MeshCollection>::create();
		Quad::createMesh(meshCollection);
		addAsAsset("SGE_MESH_QUAD", meshCollection);
	}

	{
		// Create sphere
		ResourceWrapper<MeshCollection> meshCollection = Factory<MeshCollection>::create();
		Sphere::createMesh(meshCollection, 1, 36, 36);
		addAsAsset("SGE_MESH_SPHERE", meshCollection);
	}

	{
		ResourceWrapper<MeshCollection> meshCollection = Factory<MeshCollection>::create();
		Grid::generateGrid(meshCollection, 10, 10);
		addAsAsset("SGE_MESH_GRID", meshCollection);
	}

	{
		ModelImportSettings aInfo;
		aInfo.aType = AssetType::MESH;
		aInfo.name = "SGE_MESH_CAMERA";
		aInfo.isEngineOwned = true;
		MeshCollection::import(SGE_ROOT_DIR + "Resources/Engine/Meshes/camera_v2.dae", aInfo);
	}
}

void BuiltInAssetsLoader::loadShaders()
{
	ShaderAssetDescriptor desc;
	desc.isEngineOwned = true;
	desc.name = SGE_SHADER_DEFFERED_PBR_GEOM;
	Shader::import(SGE_ROOT_DIR + "Resources/Engine/Shaders/PBR_GeomPassShader.glsl", desc);
}

void BuiltInAssetsLoader::loadAssets()
{
	loadTextures();
	loadMaterials();
	loadMeshes();
	loadShaders();
}

