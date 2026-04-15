#pragma once

#include <map>
#include <memory>
#include "core/Core.h"
#include "memory/ResourceRef.h"
#include "memory/AssetAliases.h"

class Model;
class Shader;
class IRenderer;
class PointLight;
class DirectionalLight;
class Skybox;
class Object3D;
class SceneAsset;
class Scene;
class SceneManager;
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

	bool addScene(const SceneAssetRef& scene);
	bool removeScene(const SceneAssetRef& scene);

	Window* getWindow() const;
	//ImguiHandler* getImguiHandler() const;
	Input* getInput() const;
	EventSystem* getEventSystem() const;
	//Resource<Material> getDefaultMaterial() const;

	SceneResourceRef getActiveScene() const;

	SceneAssetRef getActiveSceneAsset() const;

	void startSimulation();

	void stopSimulation();

	ProjectAssetRegistry* getProjectAssetRegistry() const;

	SGE_Regsitry& getRegistry() const;
	

	void setActiveScene(uint32_t index);

	//Resource<Texture> getDummyTexture();

	const std::map<uint32_t, SceneResourceRef>& getAllScenes() const;
	uint32_t getActiveSceneID() const;

	SceneManager* getSceneManager() const;

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

	std::shared_ptr<SceneManager> m_sceneManager;

	std::shared_ptr<ProjectAssetRegistry> m_projectAssetRegistry;

	std::shared_ptr<SGE_Regsitry> m_orphanRegistry;
};


