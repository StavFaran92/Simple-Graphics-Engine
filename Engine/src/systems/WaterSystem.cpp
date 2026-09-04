#include "systems/WaterSystem.h"

#include "core/Engine.h"
#include "memory/Assets.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "geometry/ModelImporter.h"
#include "component/MeshRendererComponent.h"
#include "component/RenderableComponent.h"
#include "component/WaterBodyComponent.h"
#include "component/Transformation.h"

WaterSystem::WaterSystem()
{
}

Entity WaterSystem::createPool()
{
	auto waterBodyEntity = Engine::get()->getContext()->getActiveScene()->createEntity("Pool");
	auto& waterBodyComponent = waterBodyEntity.addComponent<WaterBodyComponent>(waterBodyEntity);

	auto waterBodyNestedImpl = Engine::get()->getContext()->getActiveScene()->createEntity("NestedImpl");
	auto& transform = waterBodyNestedImpl.getComponent<Transformation>();
	transform.scale(glm::vec3{ 10, 10, 1 });
	transform.rotate(glm::vec3{ 3.14 / 2.f, 0, 0 });
	waterBodyNestedImpl.setParent(waterBodyEntity);

	// TODO use grid instead
	AssetBuildDescriptor meshDesc;
	meshDesc.aType = AssetType::MODEL;
	meshDesc.isEngineOwned = true;
	ModelLoadDescriptor meshLoadDesc;
	meshLoadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Meshes/sd_plane.fbx";
	auto mesh = Engine::get()->getSubSystem<Assets>()->importAsset(meshDesc, meshLoadDesc).as<ModelAsset>();
	auto& meshRendererComponent = waterBodyNestedImpl.addComponent<MeshRendererComponent>(mesh);

	AssetBuildDescriptor shaderDesc;
	shaderDesc.aType = AssetType::SHADER;
	shaderDesc.name = "WaterShader";
	shaderDesc.isEngineOwned = true;
	ShaderLoadDescriptor shaderLoadDesc;
	shaderLoadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Shaders/WaterShader.glsl";
	shaderLoadDesc.shaderOverride = ShaderOverride::PBR;
	auto shaderAsset = Engine::get()->getSubSystem<Assets>()->importAsset(shaderDesc, shaderLoadDesc).as<ShaderAsset>();

	AssetBuildDescriptor materialDesc;
	materialDesc.aType = AssetType::MATERIAL;
	materialDesc.name = "WaterMaterial";
	materialDesc.isEngineOwned = true;
	// TODO: MaterialCreateDescriptor with renderMode = Custom, customShader = shaderAsset
	MaterialCreateDescriptor materialCreateDesc;
	materialCreateDesc.data.setMaterialRenderMode(MaterialRenderMode::Custom);
	materialCreateDesc.data.setCustomShader(shaderAsset);
	auto materialAsset = Engine::get()->getSubSystem<Assets>()->createAsset(materialDesc, materialCreateDesc).as<MaterialAsset>();

	meshRendererComponent.setMaterial(0, materialAsset);

	AssetBuildDescriptor waterNormalDesc;
	waterNormalDesc.aType = AssetType::TEXTURE;
	TextureLoadDescriptor waterNormalLoadDesc;
	waterNormalLoadDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Textures/water_new_height.png";
	auto waterNormal = Engine::get()->getSubSystem<Assets>()->importAsset(waterNormalDesc, waterNormalLoadDesc).as<TextureAsset>();
	auto waterNormalSampler = std::make_shared<TextureSamplerAsset>(1);
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

	if (materialResource.isEmpty())
	{
		logError("WaterBody Resource is empty.");
		return;
	}

	materialResource->setSampler("uWaterNormalSampler", waterBody.waterBodyNormal->resolve());

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
