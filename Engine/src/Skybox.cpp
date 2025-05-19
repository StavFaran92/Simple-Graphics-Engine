#include "Skybox.h"

#include <GL/glew.h>

#include "Vertex.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Box.h"
#include "Resource.h"
#include "Shader.h"
#include "ShapeFactory.h"
#include "Material.h"
#include "Component.h"
#include "Context.h"
#include "Assets.h"
#include "Engine.h"
#include "Scene.h"
#include "Cubemap.h"
#include "CacheSystem.h"

#include "cube.h"
#include "EquirectangularToCubemapConverter.h"
#include "TextureTransformer.h"
#include "IBL.h"

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
    entity.RemoveComponent<MaterialComponent>();
    auto& skyboxComponent = entity.addComponent<SkyboxComponent>(cubemap);
    skyboxComponent.originalImage = equirectangularMap;

    entity.getComponent<ObjectComponent>().name = "Skybox";

    return entity;
}
