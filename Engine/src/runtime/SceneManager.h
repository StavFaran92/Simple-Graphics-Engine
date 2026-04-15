#pragma once

#include <map>
#include <memory>
#include "core/Core.h"
#include "memory/ResourceWrapper.h"
#include "memory/AssetAliases.h"
#include "serialize/Archiver.h"

class SceneAsset;
class Scene;
struct SerializedScene;

class EngineAPI SceneManager
{
public:
	SceneManager();

	bool addScene(const SceneAssetRef& scene);
	bool removeScene(const SceneAssetRef& scene);

	SceneResourceRef getActiveScene() const;
	SceneAssetRef getActiveSceneAsset() const;

	void setActiveScene(uint32_t index);

	const std::map<uint32_t, SceneResourceRef>& getAllScenes() const;
	uint32_t getActiveSceneID() const;

	void startSimulation();
	void stopSimulation();

	void close();

private:
	int m_activeScene = 0;
	uint32_t m_scenesCounter = 0;
	std::map<uint32_t, SceneAssetRef> m_scenes;
	std::map<uint32_t, SceneResourceRef> m_scenesCache; // Cache for getAllScenes()
	SerializedScene m_serializedScene;
	//std::stringstream m_serializedScene;
};
