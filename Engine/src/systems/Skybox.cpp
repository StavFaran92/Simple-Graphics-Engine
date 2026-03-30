#include "systems/Skybox.h"

#include "geometry/Box.h"
#include "memory/ResourceWrapper.h"
#include "memory/Assets.h"
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
        AssetBuildDescriptor texDesc;
        texDesc.aType = AssetType::TEXTURE;
        TextureLoadDescriptor loadDesc;
        loadDesc.sourcePath = textureFilepath;
        auto skyboxTexture = Engine::get()->getSubSystem<Assets>()->importAsset(texDesc, loadDesc).as<TextureAsset>();
        auto& skyboxComponent = skyboxEntity.addComponent<SkyboxComponent>(skyboxTexture);
    }

    return skyboxEntity;
    
}

Entity Skybox::createSkyboxHelper(AssetHandle<TextureAsset> cubemap, AssetHandle<TextureAsset> equirectangularMap, Entity& entity, Scene* scene)
{
    if (!scene)
    {
        scene = Engine::get()->getContext()->getActiveScene().get();
    }

    // Create irradiance map using created cubemap
    auto irradianceMap = IBL::generateIrradianceMap(cubemap.resource());

    // Create prefilter env map using created cubemap
    auto prefilterEnvMap = IBL::generatePrefilterEnvMap(cubemap.resource());

    scene->setIBLData(irradianceMap, prefilterEnvMap);

    entity.RemoveComponent<RenderableComponent>();
    //entity.RemoveComponent<MaterialComponent>();
    auto& skyboxComponent = entity.addComponent<SkyboxComponent>(cubemap);
    skyboxComponent.originalImage = equirectangularMap;

    entity.getComponent<ObjectComponent>().name = "Skybox";

    return entity;
}
