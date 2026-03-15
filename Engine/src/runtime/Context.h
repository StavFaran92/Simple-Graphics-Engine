#pragma once

#include <map>
#include <memory>
#include "core/Core.h"
#include "memory/ResourceWrapper.h"
#include "memory/AssetHandle.h"

class Model;
class Shader;
class IRenderer;
class PointLight;
class DirectionalLight;
class Skybox;
class Object3D;
class SceneAsset;
class Scene;
class Engine;
class PhongShader;
class PickingShader;
class Window;
class ImguiHandler;
class Input;
class EventSystem;
class ModelImporter;
class TextureHandler;
class Material;
class ProjectAssetRegistry;
class SGE_Regsitry;
class Archiver;
struct SerializedScene;
enum class RenderMode;


class EngineAPI Context
{
public:
	Context(const std::shared_ptr<ProjectAssetRegistry>& par);

	bool addScene(const AssetHandle<SceneAsset>& scene);
	bool removeScene(const AssetHandle<SceneAsset>& scene);

	Window* getWindow() const;
	//ImguiHandler* getImguiHandler() const;
	Input* getInput() const;
	EventSystem* getEventSystem() const;
	//Resource<Material> getDefaultMaterial() const;

	ResourceWrapper<Scene> getActiveScene() const;

	AssetHandle<SceneAsset> getActiveSceneAsset() const;

	void startSimulation();

	void stopSimulation();

	ProjectAssetRegistry* getProjectAssetRegistry() const;

	SGE_Regsitry& getRegistry() const;
	

	void setActiveScene(uint32_t index);

	//Resource<Texture> getDummyTexture();

	const std::map<uint32_t, ResourceWrapper<Scene>>& getAllScenes() const;
	uint32_t getActiveSceneID() const;

	void save() const;

	void close();

	void setRenderMode(RenderMode mode);
	RenderMode getRenderMode() const;

private:
	friend class Engine;
	friend class Archiver;
	void update(float deltaTime);
	void draw(float deltaTime);
	void init();

	int m_activeScene = -1;
	uint32_t m_scenesCounter = 0;
	std::map<uint32_t, AssetHandle<SceneAsset>> m_scenes;

	std::shared_ptr<ProjectAssetRegistry> m_projectAssetRegistry;

	std::shared_ptr<SGE_Regsitry> m_orphanRegistry;

	std::shared_ptr<SerializedScene> m_serializedScene;
};


