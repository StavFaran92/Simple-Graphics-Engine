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
	cubemap = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(flippedImageGammeCorrected);

	cubemapIBL = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(flippedImage);
	cubemapIBL->generateMipMaps();

	auto irradianceMap = IBL::generateIrradianceMap(cubemapIBL);
	auto prefilterEnvMap = IBL::generatePrefilterEnvMap(cubemapIBL);

	m_scene->setIBLData(irradianceMap, prefilterEnvMap);
}

void SkyboxComponent::resolve(ResourceWrapper<Scene>& scene)
{
	m_scene = scene;

	build();
}
