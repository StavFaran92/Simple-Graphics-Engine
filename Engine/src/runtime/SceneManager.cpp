#include "runtime/SceneManager.h"

#include "runtime/Scene.h"
#include "memory/AssetRef.h"
#include "serialize/Archiver.h"
#include "core/Logger.h"
#include "serialize/CerealHelpers.h"
#include "component/ComponentSerializer.h"

SceneManager::SceneManager()
{
	//m_serializedScene = std::make_shared<SerializedScene>();
}

bool SceneManager::addScene(const SceneAssetRef& sceneAsset)
{
	auto scene = sceneAsset.resource();
	
	scene->SetID(m_scenesCounter);
	m_scenes[m_scenesCounter] = sceneAsset;
	m_scenesCache[m_scenesCounter] = scene;
	m_scenesCounter += 1;

	logInfo("Scene {} Added successfully.", std::to_string(m_scenesCounter));


	return true;
}

bool SceneManager::removeScene(const SceneAssetRef& scene)
{
	return false;
}

SceneResourceRef SceneManager::getActiveScene() const
{
	if (m_activeScene == -1 || m_activeScene >= m_scenes.size())
		return SceneResourceRef::empty;

	return m_scenes.at(m_activeScene).resource();
}

SceneAssetRef SceneManager::getActiveSceneAsset() const
{
	if (m_activeScene == -1)
		return SceneAssetRef::empty;

	return m_scenes.at(m_activeScene);
}

void SceneManager::setActiveScene(uint32_t index)
{
	if (index > m_scenesCounter)
	{
		logError("Illegal index specified: " + std::to_string(index));
		return;
	}

	if (m_activeScene != -1)
		m_scenes.at(m_activeScene).resource()->onDeactivate();

	m_activeScene = index;

	m_scenes.at(m_activeScene).resource()->onActivate();
}

const std::map<uint32_t, SceneResourceRef>& SceneManager::getAllScenes() const
{
	return m_scenesCache;
}

uint32_t SceneManager::getActiveSceneID() const
{
	return m_activeScene;
}

void SceneManager::startSimulation()
{
	auto activeScene = getActiveScene();
	if (activeScene.isEmpty())
		return;

	m_serializedScene = Archiver::serializeScene(activeScene);

	activeScene->startSimulation();
}

void SceneManager::stopSimulation()
{
	auto activeScene = getActiveScene();
	if (activeScene.isEmpty())
		return;

	activeScene->stopSimulation();

	Archiver::deserializeScene(m_serializedScene, activeScene);

	for (auto& cbWrapper : ComponentSerdes::getRegistry())
	{
		activeScene->getRegistry().get().each([&](entt::entity e) {
			Entity entity(e, &activeScene->getRegistry());
			cbWrapper.resolve(entity, activeScene);
		});

	}

	for (auto& cbWrapper : ComponentSerdes::getRegistry())
	{
		activeScene->getRegistry().get().each([&](entt::entity e) {
			Entity entity(e, &activeScene->getRegistry());
			cbWrapper.postLoad(entity, activeScene);
		});
	}
}

void SceneManager::close()
{
	m_scenes.clear();
	m_scenesCache.clear();
}
