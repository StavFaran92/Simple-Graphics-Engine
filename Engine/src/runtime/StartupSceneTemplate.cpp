#include "StartupSceneTemplate.h"

#include "core/Engine.h"
#include "core/Window.h"
#include "runtime/Context.h"
#include "component/Transformation.h"
#include "component/CameraComponent.h"
#include "component/MeshRendererComponent.h"
#include "lights/DirectionalLight.h"
#include "memory/BuiltInAssets.h"

SceneAssetRef StartupSceneTemplate::createStartupScene(const std::string& name)
{
    AssetBuildDescriptor desc;
    desc.name = name;
    desc.aType = AssetType::SCENE;
    SceneCreateDescriptor sceneDesc;
    SceneAssetRef sceneAsset = Engine::get()->getSubSystem<Assets>()->createAsset(desc, sceneDesc).as<SceneAsset>();

    auto startupScene = sceneAsset.resource();

    // Add default dir light
    auto dLight = startupScene->createEntity("Directional light");
    dLight.addComponent<DirectionalLight>();
    dLight.getComponent<Transformation>().setLocalRotation(glm::vec3(0, -1, 0));

    auto mainCamera = startupScene->createEntity("Main Camera");
    mainCamera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)Engine::get()->getWindow()->getWidth() / Engine::get()->getWindow()->getHeight(), 0.1f, 1000.0f));
    mainCamera.getComponent<Transformation>().setLocalPosition({ 10,10,10 });
    mainCamera.getComponent<CameraComponent>().center = { 0,0,0 };
    mainCamera.getComponent<CameraComponent>().up = { 0,1,0 };
    mainCamera.addComponent<MeshRendererComponent>(BuiltInAssets::getByName<ModelAsset>(SGE_MESH_CAMERA));

    startupScene->setGameCamera(mainCamera);

    Engine::get()->getContext()->addScene(sceneAsset);

    return sceneAsset;
}