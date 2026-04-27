#include "runtime/Context.h"

#include "lights/DirectionalLight.h"
#include "lights/PointLight.h"
#include "camera/ICamera.h"
#include "render/Shader.h"

#include "runtime/SceneManager.h"
#include "runtime/Scene.h"
#include "core/Engine.h"

#include "core/Logger.h"
#include "core/Engine.h"
#include "texture/Texture.h"
#include "render/Material.h"
#include "serialize/ProjectAssetRegistry.h"
#include "render/Graphics.h"
#include "memory/Assets.h"

Context::Context(const std::shared_ptr<ProjectAssetRegistry>& par)
{
	m_projectAssetRegistry = par;

	m_orphanRegistry = std::make_shared<SGE_Regsitry>();

	m_sceneManager = std::make_shared<SceneManager>();
}

void Context::init()
{
	
}

bool Context::addScene(const SceneAssetRef& sceneAsset)
{
	return m_sceneManager->addScene(sceneAsset);
}

bool Context::removeScene(const SceneAssetRef& scene)
{
	return m_sceneManager->removeScene(scene);
}

SceneResourceRef Context::getActiveScene() const
{
	return m_sceneManager->getActiveScene();
}

SceneAssetRef Context::getActiveSceneAsset() const
{
	return m_sceneManager->getActiveSceneAsset();
}

void Context::startSimulation()
{
	m_sceneManager->startSimulation();
}

void Context::stopSimulation()
{
	m_sceneManager->stopSimulation();
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
	m_sceneManager->setActiveScene(index);
}

//Resource<Texture> Context::getDummyTexture()
//{
//	return m_dummyTexture;
//}

const std::map<uint32_t, SceneResourceRef>& Context::getAllScenes() const
{
	return m_sceneManager->getAllScenes();
}

uint32_t Context::getActiveSceneID() const
{
	return m_sceneManager->getActiveSceneID();
}

SceneManager* Context::getSceneManager() const
{
	return m_sceneManager.get();
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
	Engine::get()->getSubSystem<Assets>()->syncAllAssets();

	auto activeScene = m_sceneManager->getActiveScene();
	if (activeScene.isEmpty())
		return;

	//if (!activeScene->isSimulationActive())
	//{
	//	activeScene->startSimulation();
	//}

	activeScene->update(deltaTime);
}

void Context::draw(float deltaTime)
{
	auto activeScene = m_sceneManager->getActiveScene();
	if (activeScene.isEmpty())
		return;

	activeScene->draw(deltaTime);
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
	m_sceneManager->close();
}