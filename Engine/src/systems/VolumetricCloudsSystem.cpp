#include "systems/VolumetricCloudsSystem.h"

#include "core/Engine.h"
#include "memory/Assets.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "geometry/ModelImporter.h"
#include "component/MeshRendererComponent.h"
#include "component/ShaderComponent.h"
#include "component/RenderableComponent.h"
#include "component/VolumetricCloudsComponent.h"
#include "component/VolumeComponent.h"
#include "memory/BuiltInAssets.h"

VolumetricCloudsSystem::VolumetricCloudsSystem()
{
	Engine::get()->registerSubSystem<VolumetricCloudsSystem>(this);

	TextureLoadDescriptor desc;
	desc.filter = TextureFilter::Linear;
	desc.wrap = TextureWrap::Mirror;
	desc.genMipMap = true;
	m_noiseTexture = Texture::load(SGE_ROOT_DIR "Resources/Engine/Textures/cloud_noise.png", desc);
}

Entity VolumetricCloudsSystem::createVolumetricClouds()
{
	auto volumetricCloudsEntity = Engine::get()->getContext()->getActiveScene()->createEntity("Volumetric Clouds");
	auto& volumetricCloudsComponent = volumetricCloudsEntity.addComponent<VolumetricCloudsComponent>(volumetricCloudsEntity);

	auto cloudVolumeEntity = Engine::get()->getContext()->getActiveScene()->createEntity("Volume");
	cloudVolumeEntity.setParent(volumetricCloudsEntity);

	auto& cloudVolumeComponent = cloudVolumeEntity.addComponent<VolumeComponent>();
	

	AssetCreateDescriptor shaderAssetDesc;
	shaderAssetDesc.aType = AssetType::SHADER;
	shaderAssetDesc.sourcePath = SGE_ROOT_DIR "Resources/Engine/Shaders/VolumetricCloudsShader.glsl";
	shaderAssetDesc.name = "VolumetricCloudsShader";
	auto* shaderDesc = shaderAssetDesc.makeResourceLoadDescriptor<ShaderLoadDescriptor>();
	shaderDesc->shaderOverride = ShaderOverride::Volume;
	auto shaderAsset = Engine::get()->getSubSystem<Assets>()->importAsset(shaderAssetDesc).as<ShaderAsset>();

	AssetCreateDescriptor materialDesc;
	materialDesc.aType = AssetType::MATERIAL;
	materialDesc.name = "cloudsMaterial";
	materialDesc.isEngineOwned = true;
	// TODO: makeResourceCreateDescriptor<MaterialCreateDescriptor>() with renderMode = Custom, customShader = shaderAsset
	auto materialAsset = Engine::get()->getSubSystem<Assets>()->createAsset(materialDesc).as<MaterialAsset>();

	cloudVolumeComponent.material = materialAsset;
	cloudVolumeComponent.mesh = BuiltInAssets::getByName<ModelAsset>(SGE_MESH_BOX);

	return volumetricCloudsEntity;
}

void VolumetricCloudsSystem::prepareVolumetricCloudsForRender(VolumetricCloudsComponent& clouds)
{
	auto& materialResource = clouds.getMaterial().resource();
	materialResource->setUniformValue("MARCH_SIZE", clouds.marchSize);


	VolumeComponent& volumeComponent = clouds.entity.getComponentInChildren<VolumeComponent>();
	auto& mesh = volumeComponent.mesh.resource()->getPrimaryMesh();

	auto& transform = clouds.entity.getComponent<Transformation>();
	glm::mat4 modelTransform = transform.getWorldTransformation() * mesh->getRestTransform();

	AABB& aabb = mesh->getAABB();
	aabb.transform(modelTransform);

	materialResource->setUniformValue("u_aabbMin", aabb.getMin());
	materialResource->setUniformValue("u_aabbMax", aabb.getMax());

	materialResource->setTexture("iChannel0", m_noiseTexture);

	//if (!aabb.isOnFrustum(*graphics->frustum))
	//{
	//	return false;
	//}
	//materialResource->setSampler("uWaterNormalSampler", waterBody.waterBodyNormal);

	//materialResource->setUniformValue("uWave1Speed", waterBody.wave1Speed);
	//materialResource->setUniformValue("uWave2Speed", waterBody.wave2Speed);
	//materialResource->setUniformValue("uWave3Speed", waterBody.wave3Speed);

	//materialResource->setUniformValue("uWave1Amp", waterBody.wave1Amp);
	//materialResource->setUniformValue("uWave2Amp", waterBody.wave2Amp);
	//materialResource->setUniformValue("uWave3Amp", waterBody.wave3Amp);

	//materialResource->setUniformValue("uColorA", waterBody.colorA);
	//materialResource->setUniformValue("uColorB", waterBody.colorB);
	//materialResource->setUniformValue("opacityFactor", waterBody.opacity);
}
