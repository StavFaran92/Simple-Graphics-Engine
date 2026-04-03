#include "component/SkyboxComponent.h"

#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "utils/EquirectangularToCubemapConverter.h"
#include "render/IBL.h"
#include "texture/TextureTransformer.h"

SkyboxComponent::SkyboxComponent(AssetHandle<TextureAsset> skyboxImage)
{
	setSkybox(skyboxImage);
}

void SkyboxComponent::setSkybox(AssetHandle<TextureAsset> image)
{
	originalImage = image;
}

void SkyboxComponent::build()
{
	// TODO check if orig image is cube and support cubemap load

	assert(!m_scene.isEmpty());

	ResourceWrapper<Texture> flippedImage = TextureTransformer::flipVertical(originalImage.resource());
	ResourceWrapper<Texture> flippedImageGammeCorrected = TextureTransformer::applyGammaCorrection(flippedImage);
	auto cubemap = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(flippedImageGammeCorrected);
	m_cubemap = Engine::get()->getSubSystem<Assets>()->bakeAssetFromResource(cubemap).as<TextureAsset>();

	cubemapIBL = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(flippedImage);
	cubemapIBL->generateMipMaps();

	auto irradianceMap = IBL::generateIrradianceMap(cubemapIBL);
	m_irradianceMap = Engine::get()->getSubSystem<Assets>()->bakeAssetFromResource(irradianceMap).as<TextureAsset>();
	
	auto prefilterEnvMap = IBL::generatePrefilterEnvMap(cubemapIBL);
	m_prefilterEnvMap = Engine::get()->getSubSystem<Assets>()->bakeAssetFromResource(prefilterEnvMap).as<TextureAsset>();

	m_isBuilt = true;

	
}

void SkyboxComponent::resolve(ResourceWrapper<Scene>& scene)
{
	m_scene = scene;
}

void SkyboxComponent::postLoad(ResourceWrapper<Scene>& scene)
{
	if (!m_isBuilt)
	{
		build();
	}

	m_scene->setIBLData(m_irradianceMap.resource(), m_prefilterEnvMap.resource());
}
