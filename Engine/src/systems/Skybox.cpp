#include "systems/Skybox.h"

#include <GL/glew.h>

#include "render/Vertex.h"
#include "render/Renderer.h"
#include "geometry/Mesh.h"
#include "geometry/Box.h"
#include "memory/Resource.h"
#include "render/Shader.h"
#include "geometry/ShapeFactory.h"
#include "render/Material.h"
#include "component/Component.h"
#include "runtime/Context.h"
#include "memory/Assets.h"
#include "core/Engine.h"
#include "runtime/Scene.h"
#include "texture/Cubemap.h"
#include "core/CacheSystem.h"

#include "geometry/cube.h"
#include "utils/EquirectangularToCubemapConverter.h"
#include "texture/TextureTransformer.h"
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
        auto skyboxTexture = Texture::importTexture2D(textureFilepath);
        auto& skyboxComponent = skyboxEntity.addComponent<SkyboxComponent>(skyboxTexture);
        skyboxComponent.build();
    }

    return skyboxEntity;
    
}

Entity Skybox::CreateSkyboxFromEquirectangularMap(const std::string& equirectnagularMapPath, Scene* scene)
{
    if (!scene)
    {
        scene = Engine::get()->getContext()->getActiveScene().get();
    }

    auto equirectnagularMap = Texture::importTexture2D(equirectnagularMapPath);

    //texture = TextureTransformer::flipVertical(texture);

    auto cubemap = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(equirectnagularMap);

    auto entity = scene->createEntity();

    return createSkyboxHelper(cubemap, equirectnagularMap, entity, scene);
}

Entity Skybox::loadSkybox(Resource<Texture> equirectnagularMap, Entity& entity, Scene* scene)
{
    if (!scene)
    {
        scene = Engine::get()->getContext()->getActiveScene().get();
    }

    //equirectnagularMap = TextureTransformer::flipVertical(equirectnagularMap);

    auto cubemap = EquirectangularToCubemapConverter::fromEquirectangularToCubemap(equirectnagularMap);

    return createSkyboxHelper(cubemap, equirectnagularMap, entity, scene);
}

Entity Skybox::CreateSkyboxFromCubemap(const SkyboxFaces& faces, Scene* scene)
{
    std::vector<std::string> facesVec{ faces.right, faces.left, faces.top, faces.bottom, faces.front, faces.back };
    auto cubemap = Cubemap::createCubemapFromCubemapFiles(facesVec);

    Resource<Texture> equirectangularMap = EquirectangularToCubemapConverter::fromCubemapToEquirectangular(cubemap);
    equirectangularMap = TextureTransformer::flipVertical(equirectangularMap);
    Cubemap::saveEquirectangularMap(equirectangularMap);
    Texture::addTexture2D(equirectangularMap);

    static int skyboxCount = 0; // TODO fix - will not work with load
    Engine::get()->getMemoryManagementSystem()->addAssociation("SKYBOX_" + std::to_string(skyboxCount++), equirectangularMap.getUID());

    auto entity = scene->createEntity();

    return loadSkybox(equirectangularMap, entity, scene);
}

Entity Skybox::createSkyboxHelper(Resource<Texture> cubemap, Resource<Texture> equirectangularMap, Entity& entity, Scene* scene)
{
    if (!scene)
    {
        scene = Engine::get()->getContext()->getActiveScene().get();
    }

    // Create irradiance map using created cubemap
    auto irradianceMap = IBL::generateIrradianceMap(cubemap, scene);

    // Create prefilter env map using created cubemap
    auto prefilterEnvMap = IBL::generatePrefilterEnvMap(cubemap, scene);

    scene->setIBLData(irradianceMap, prefilterEnvMap);

    entity.RemoveComponent<RenderableComponent>();
    //entity.RemoveComponent<MaterialComponent>();
    auto& skyboxComponent = entity.addComponent<SkyboxComponent>(cubemap);
    skyboxComponent.originalImage = equirectangularMap;

    entity.getComponent<ObjectComponent>().name = "Skybox";

    return entity;
}
