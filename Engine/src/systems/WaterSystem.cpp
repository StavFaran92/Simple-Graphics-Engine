#include "systems/WaterSystem.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "geometry/ModelImporter.h"
#include "component/MeshRendererComponent.h"
#include "component/ShaderComponent.h"
#include "component/RenderableComponent.h"

Entity WaterSystem::createPool()
{
	auto poolQuad = Engine::get()->getContext()->getActiveScene()->createEntity("Pool");

	// TODO use grid instead
	ModelImportSettings aDesc;
	aDesc.isEngineOwned = true;
	auto& meshRendererComponent = poolQuad.addComponent<MeshRendererComponent>(MeshCollection::import(SGE_ROOT_DIR + "Resources/Engine/Meshes/sd_plane.fbx", aDesc));

	auto& shader = Shader::createOverrideShader(SGE_ROOT_DIR + "Resources/Engine/Shaders/WaterShader.glsl", ShaderOverride::PBR);

	AssetCreateDescriptor shaderDesc;
	shaderDesc.aType = AssetType::SHADER;
	shaderDesc.origFilePath = SGE_ROOT_DIR + "Resources/Engine/Shaders/WaterShader.glsl";
	shaderDesc.name = "WaterShader";
	shaderDesc.attributes[Shader::ATTRIB_SHADER_OVERRIDE] = Shader::getShaderOverrideAsStr(ShaderOverride::PBR);
	shaderDesc.isEngineOwned = true;
	auto& shaderAsset = Engine::get()->getSubSystem<Assets>()->createAsset(shader, shaderDesc).as<Shader>();

	auto& material = Material::create(MaterialRenderMode::Custom, shaderAsset);
	AssetCreateDescriptor materialDesc;
	materialDesc.aType = AssetType::MATERIAL;
	materialDesc.name = "WaterMaterial";
	materialDesc.isEngineOwned = true;
	auto& materialAsset = Engine::get()->getSubSystem<Assets>()->createAsset(material, materialDesc).as<Material>();

	meshRendererComponent.setMaterial(0, materialAsset);

	//auto& shaderComponent = poolQuad.addComponent<ShaderComponent>();
	//shaderComponent.setShader(shaderAsset);

	auto waterNormal = Texture::import(SGE_ROOT_DIR + "Resources/Engine/Textures/water_new_height.png");
	auto waterNormalSampler = std::make_shared<TextureSampler>(1);
	waterNormalSampler->texture = waterNormal;
	materialAsset.get()->setSampler("waterNormalSampler", waterNormalSampler);

	return poolQuad;
}
