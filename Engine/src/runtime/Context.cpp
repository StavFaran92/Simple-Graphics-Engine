#include "runtime/Context.h"

#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"
#include "camera/ICamera.h"
#include "render/Shader.h"

#include "runtime/Scene.h"
#include "core/Engine.h"

#include "core/Logger.h"
#include "core/Engine.h"
#include "texture/Texture.h"
#include "render/Material.h"
#include "serialize/ProjectAssetRegistry.h"
#include "render/Graphics.h"

Context::Context(const std::shared_ptr<ProjectAssetRegistry>& par)
{
	m_projectAssetRegistry = par;

	m_orphanRegistry = std::make_shared<SGE_Regsitry>();

	m_serializedScene = std::make_shared<SerializedScene>();
}

void Context::init()
{
	
}

bool Context::addScene(const AssetHandle<SceneAsset>& sceneAsset)
{
	auto scene = sceneAsset.resource();
	m_scenesCounter += 1;
	scene->SetID(m_scenesCounter);
	m_scenes[m_scenesCounter] = sceneAsset;

	logInfo("Scene {} Added successfully.", std::to_string(m_scenesCounter));

	return true;
}

bool Context::removeScene(const AssetHandle<SceneAsset>& scene)
{
	return false;
}

ResourceWrapper<Scene> Context::getActiveScene() const
{
	if (m_activeScene == -1)
		return nullptr;

	return m_scenes.at(m_activeScene).resource();
}

AssetHandle<SceneAsset> Context::getActiveSceneAsset() const
{
	if (m_activeScene == -1)
		return AssetHandle<SceneAsset>::empty;

	return m_scenes.at(m_activeScene);
}

void Context::startSimulation()
{
	auto activeScene = getActiveScene();

	*m_serializedScene = Archiver::serializeScene(activeScene);
	activeScene->startSimulation();
}

void Context::stopSimulation()
{
	auto activeScene = getActiveScene();

	activeScene->stopSimulation();
	Archiver::deserializeScene(*m_serializedScene, activeScene);
}

ProjectAssetRegistry* Context::getProjectAssetRegistry() const
{
	return m_projectAssetRegistry.get();
}

SGE_Regsitry& Context::getRegistry() const
{
	return *m_orphanRegistry.get();
}

void Context::setActiveScene(uint32_t index)
{
	if (index > m_scenesCounter)
	{
		logError("Illegal index specified: " + index);
		return;
	}

	m_activeScene = index;
}

//Resource<Texture> Context::getDummyTexture()
//{
//	return m_dummyTexture;
//}

const std::map<uint32_t, ResourceWrapper<Scene>>& Context::getAllScenes() const
{
	return {}; // todo fix
}

uint32_t Context::getActiveSceneID() const
{
	return m_activeScene;
}

void Context::save() const
{
	m_projectAssetRegistry->save();

	logInfo("Successfully serialized Context.");
}

void Context::setRenderMode(RenderMode mode)
{
	Engine::get()->getSubSystem<Graphics>()->renderMode = mode;
}

RenderMode Context::getRenderMode() const
{
	return Engine::get()->getSubSystem<Graphics>()->renderMode;
}

void Context::update(float deltaTime)
{
	if (m_activeScene == -1)
		return;

	//if (!m_scenes[m_activeScene]->isSimulationActive())
	//{
	//	m_scenes[m_activeScene]->startSimulation();
	//}

	m_scenes[m_activeScene].resource()->update(deltaTime); // todo fix
}

void Context::draw(float deltaTime)
{
	if (m_activeScene == -1)
		return;

	m_scenes[m_activeScene].resource()->draw(deltaTime); // todo fix
}

Window* Context::getWindow() const
{
	return Engine::get()->getWindow();
}

//ImguiHandler* Context::getImguiHandler() const
//{
//	return Engine::get()->getImguiHandler();
//}

Input* Context::getInput() const
{
	return Engine::get()->getInput();
}

EventSystem* Context::getEventSystem() const
{
	return Engine::get()->getEventSystem();
}

//Resource<Material> Context::getDefaultMaterial() const
//{
//	return m_defaultMaterial;
//}

void Context::close()
{
	m_scenes.clear();
}