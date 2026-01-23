#include "systems/Skybox.h"

#include "geometry/Box.h"
#include "memory/ResourceWrapper.h"
#include "runtime/Context.h"
#include "core/Engine.h"
#include "runtime/Scene.h"

#include "render/IBL.h"
#include "component/SkyboxComponent.h"
#include "component/RenderableComponent.h"
#include "component/ObjectComponent.h"

Entity Skybox::createSkybox(const std::string& textureFilepath, TexType texType)
{
    auto scene = Engine::get()->getContext()->getActiveScene();
    auto skyboxEntity = scene->createEntity("Skybox");

    // Todo support cubemap
    if (texType == TexType::CUBEMAP)
    {
        logError("Cubemap currently not supported.");
        return Entity::EmptyEntity;
    }
    else if (texType == TexType::EQUIRECTANGULAR)
    {
        auto skyboxTexture = Texture::import(textureFilepath);
        auto& skyboxComponent = skyboxEntity.addComponent<SkyboxComponent>(skyboxTexture);
        skyboxComponent.build();
    }

    return skyboxEntity;
    
}

Entity Skybox::createSkyboxHelper(AssetHandle<Texture> cubemap, AssetHandle<Texture> equirectangularMap, Entity& entity, Scene* scene)
{
    if (!scene)
    {
        scene = Engine::get()->getContext()->getActiveScene().get();
    }

    // Create irradiance map using created cubemap
    auto irradianceMap = IBL::generateIrradianceMap(cubemap.resource(), scene);

    // Create prefilter env map using created cubemap
    auto prefilterEnvMap = IBL::generatePrefilterEnvMap(cubemap.resource(), scene);

    scene->setIBLData(irradianceMap, prefilterEnvMap);

    entity.RemoveComponent<RenderableComponent>();
    //entity.RemoveComponent<MaterialComponent>();
    auto& skyboxComponent = entity.addComponent<SkyboxComponent>(cubemap);
    skyboxComponent.originalImage = equirectangularMap;

    entity.getComponent<ObjectComponent>().name = "Skybox";

    return entity;
}
