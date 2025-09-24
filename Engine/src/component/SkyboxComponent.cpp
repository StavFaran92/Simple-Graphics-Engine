#include "component/SkyboxComponent.h"

#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "utils/EquirectangularToCubemapConverter.h"
#include "render/IBL.h"

void SkyboxComponent::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	if (auto sc = std::dynamic_pointer_cast<SkyboxComponent>(c))
	{
		auto& skyboxComponent = entityHandler.addComponent<SkyboxComponent>(*sc);
		skyboxComponent.build();
	}
}

SkyboxComponent::SkyboxComponent(ResourceWrapper<Texture> skyboxImage)
{
	setSkybox(skyboxImage);
}

void SkyboxComponent::setSkybox(ResourceWrapper<Texture> image)
{
	originalImage = image;
}

void SkyboxComponent::build()
{

	// TODO check if orig image is cube and support cubemap load

	cubemap = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(originalImage);

	auto scene = Engine::get()->getContext()->getActiveScene().get();
	auto irradianceMap = IBL::generateIrradianceMap(cubemap, scene);
	auto prefilterEnvMap = IBL::generatePrefilterEnvMap(cubemap, scene);

	scene->setIBLData(irradianceMap, prefilterEnvMap);
}