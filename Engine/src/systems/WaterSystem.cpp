#include "systems/WaterSystem.h"

#include "core/Engine.h"
#include "memory/Assets.h"
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
	AssetBuildDescriptor meshDesc;
	meshDesc.aType = AssetType::MESH;
	meshDesc.isEngineOwned = true;
	meshDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Meshes/sd_plane.fbx";
	ModelLoadDescriptor meshLoadDesc;
	meshLoadDesc.aType = meshDesc.aType;
	meshLoadDesc.sourcePath = meshDesc.sourcePath;
	auto mesh = Engine::get()->getSubSystem<Assets>()->importAsset(meshDesc, meshLoadDesc).as<ModelAsset>();
	auto& meshRendererComponent = waterBodyNestedImpl.addComponent<MeshRendererComponent>(mesh);

	AssetBuildDescriptor shaderDesc;
	shaderDesc.aType = AssetType::SHADER;
	shaderDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Shaders/WaterShader.glsl";
	shaderDesc.name = "WaterShader";
	shaderDesc.isEngineOwned = true;
	ShaderLoadDescriptor shaderLoadDesc;
	shaderLoadDesc.aType = shaderDesc.aType;
	shaderLoadDesc.sourcePath = shaderDesc.sourcePath;
	shaderLoadDesc.shaderOverride = ShaderOverride::PBR;
	auto shaderAsset = Engine::get()->getSubSystem<Assets>()->importAsset(shaderDesc, shaderLoadDesc).as<ShaderAsset>();

	AssetBuildDescriptor materialDesc;
	materialDesc.aType = AssetType::MATERIAL;
	materialDesc.name = "WaterMaterial";
	materialDesc.isEngineOwned = true;
	// TODO: MaterialCreateDescriptor with renderMode = Custom, customShader = shaderAsset
	MaterialCreateDescriptor materialCreateDesc;
	materialCreateDesc.aType = materialDesc.aType;
	auto materialAsset = Engine::get()->getSubSystem<Assets>()->createAsset(materialDesc, materialCreateDesc).as<MaterialAsset>();

	meshRendererComponent.setMaterial(0, materialAsset);

	AssetBuildDescriptor waterNormalDesc;
	waterNormalDesc.aType = AssetType::TEXTURE;
	waterNormalDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Textures/water_new_height.png";
	TextureLoadDescriptor waterNormalLoadDesc;
	waterNormalLoadDesc.aType = waterNormalDesc.aType;
	waterNormalLoadDesc.sourcePath = waterNormalDesc.sourcePath;
	auto waterNormal = Engine::get()->getSubSystem<Assets>()->importAsset(waterNormalDesc, waterNormalLoadDesc).as<TextureAsset>();
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
	materialResource->setUniformValue("opacityFactor", waterBody.opacity);
}
