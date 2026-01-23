#include "component/SkyboxComponent.h"

#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "utils/EquirectangularToCubemapConverter.h"
#include "render/IBL.h"
#include "texture/TextureTransformer.h"

void SkyboxComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto sc = std::dynamic_pointer_cast<SkyboxComponent>(c))
	{
		auto& skyboxComponent = entityHandler.addComponent<SkyboxComponent>(*sc);
		skyboxComponent.build();
	}
}

SkyboxComponent::SkyboxComponent(AssetHandle<Texture> skyboxImage)
{
	setSkybox(skyboxImage);
}

void SkyboxComponent::setSkybox(AssetHandle<Texture> image)
{
	originalImage = image;
}

void SkyboxComponent::build()
{
	// TODO check if orig image is cube and support cubemap load

	ResourceWrapper<Texture> flippedImage = TextureTransformer::flipVertical(originalImage.resource());
	ResourceWrapper<Texture> flippedImageGammeCorrected = TextureTransformer::applyGammaCorrection(flippedImage);
	cubemap = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(flippedImageGammeCorrected);

	cubemapIBL = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(flippedImage);
	cubemapIBL->generateMipMaps();

	auto scene = Engine::get()->getContext()->getActiveScene().get();
	auto irradianceMap = IBL::generateIrradianceMap(cubemapIBL, scene);
	auto prefilterEnvMap = IBL::generatePrefilterEnvMap(cubemapIBL, scene);

	scene->setIBLData(irradianceMap, prefilterEnvMap);

	

}