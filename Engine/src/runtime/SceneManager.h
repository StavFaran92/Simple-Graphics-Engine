#pragma once

#include <map>
#include <memory>
#include "core/Core.h"
#include "memory/ResourceWrapper.h"
#include "memory/AssetHandle.h"

class SceneAsset;
class Scene;
struct SerializedScene;

class EngineAPI SceneManager
{
public:
	SceneManager();

	bool addScene(const AssetHandle<SceneAsset>& scene);
	bool removeScene(const AssetHandle<SceneAsset>& scene);

	ResourceWrapper<Scene> getActiveScene() const;
	AssetHandle<SceneAsset> getActiveSceneAsset() const;

	void setActiveScene(uint32_t index);

	const std::map<uint32_t, ResourceWrapper<Scene>>& getAllScenes() const;
	uint32_t getActiveSceneID() const;

	void startSimulation();
	void stopSimulation();

	void close();

private:
	int m_activeScene = 0;
	uint32_t m_scenesCounter = 0;
	std::map<uint32_t, AssetHandle<SceneAsset>> m_scenes;
	std::map<uint32_t, ResourceWrapper<Scene>> m_scenesCache; // Cache for getAllScenes()
	std::shared_ptr<SerializedScene> m_serializedScene;
};
