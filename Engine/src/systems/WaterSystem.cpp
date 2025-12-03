#include "systems/WaterSystem.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "geometry/ModelImporter.h"
#include "component/MeshRendererComponent.h"
#include "component/ShaderComponent.h"
#include "component/RenderableComponent.h"
#include "component/WaterBodyComponent.h"

WaterSystem::WaterSystem()
{
	Engine::get()->registerSubSystem<WaterSystem>(this);
}

Entity WaterSystem::createPool()
{
	auto waterBodyEntity = Engine::get()->getContext()->getActiveScene()->createEntity("Pool");
	auto& waterBodyComponent = waterBodyEntity.addComponent<WaterBodyComponent>(waterBodyEntity);

	auto waterBodyNestedImpl = Engine::get()->getContext()->getActiveScene()->createEntity("NestedImpl");
	waterBodyNestedImpl.setParent(waterBodyEntity);

	// TODO use grid instead
	ModelImportSettings meshDesc;
	meshDesc.isEngineOwned = true;
	AssetWrapper<MeshCollection> mesh = MeshCollection::import(SGE_ROOT_DIR + "Resources/Engine/Meshes/sd_plane.fbx", meshDesc);
	auto& meshRendererComponent = waterBodyNestedImpl.addComponent<MeshRendererComponent>(mesh);

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
	waterBodyComponent.waterBodyNormal = waterNormalSampler;

	return waterBodyEntity;
}

void WaterSystem::drawWaterBody(const WaterBodyComponent& waterBody)
{
}

void WaterSystem::prepareWaterBodyForRender(WaterBodyComponent& waterBody)
{
	auto& materialResource = waterBody.getMaterial().resource();
	materialResource->setSampler("uWaterNormalSampler", waterBody.waterBodyNormal);

	materialResource->setUniformValue("uWave1Speed", waterBody.wave1Speed);
	materialResource->setUniformValue("uWave2Speed", waterBody.wave2Speed);
	materialResource->setUniformValue("uWave3Speed", waterBody.wave3Speed);

	materialResource->setUniformValue("uWave1Amp", waterBody.wave1Amp);
	materialResource->setUniformValue("uWave2Amp", waterBody.wave2Amp);
	materialResource->setUniformValue("uWave3Amp", waterBody.wave3Amp);

	materialResource->setUniformValue("uColorA", waterBody.colorA);
	materialResource->setUniformValue("uColorB", waterBody.colorB);
}
