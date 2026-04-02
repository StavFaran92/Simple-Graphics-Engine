#include "runtime/SceneManager.h"

#include "runtime/Scene.h"
#include "memory/AssetHandle.h"
#include "serialize/Archiver.h"
#include "core/Logger.h"
#include "serialize/CerealHelpers.h"
#include "component/ComponentSerializer.h"

SceneManager::SceneManager()
{
}

bool SceneManager::addScene(const AssetHandle<SceneAsset>& sceneAsset)
{
	auto scene = sceneAsset.resource();
	
	scene->SetID(m_scenesCounter);
	m_scenes[m_scenesCounter] = sceneAsset;
	m_scenesCache[m_scenesCounter] = scene;
	m_scenesCounter += 1;

	logInfo("Scene {} Added successfully.", std::to_string(m_scenesCounter));


	return true;
}

bool SceneManager::removeScene(const AssetHandle<SceneAsset>& scene)
{
	return false;
}

ResourceWrapper<Scene> SceneManager::getActiveScene() const
{
	if (m_activeScene == -1 || m_activeScene >= m_scenes.size())
		return ResourceWrapper<Scene>::empty;

	return m_scenes.at(m_activeScene).resource();
}

AssetHandle<SceneAsset> SceneManager::getActiveSceneAsset() const
{
	if (m_activeScene == -1)
		return AssetHandle<SceneAsset>::empty;

	return m_scenes.at(m_activeScene);
}

void SceneManager::setActiveScene(uint32_t index)
{
	if (index > m_scenesCounter)
	{
		logError("Illegal index specified: " + std::to_string(index));
		return;
	}

	m_activeScene = index;

	m_scenes.at(m_activeScene).resource()->onActivate();
}

const std::map<uint32_t, ResourceWrapper<Scene>>& SceneManager::getAllScenes() const
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

	m_serializedScene = activeScene->getRegistry().toStream();

	activeScene->startSimulation();
}

void SceneManager::stopSimulation()
{
	auto activeScene = getActiveScene();
	if (activeScene.isEmpty())
		return;

	activeScene->stopSimulation();

	activeScene->getRegistry().fromStream(m_serializedScene);

	for (auto& cbWrapper : ComponentSerdes::getRegistry())
	{
		cbWrapper.resolve(activeScene);
	}

	for (auto& cbWrapper : ComponentSerdes::getRegistry())
	{
		cbWrapper.postLoad(activeScene);
	}
}

void SceneManager::close()
{
	m_scenes.clear();
	m_scenesCache.clear();
}
