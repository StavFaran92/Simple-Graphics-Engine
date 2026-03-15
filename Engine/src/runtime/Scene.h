#pragma once

#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <map>
#include <functional>

#include "core/Core.h"

#include "glm/glm.hpp"
#include "runtime/Entity.h"
#include "serialize/Archiver.h"
#include "memory/AssetDescriptors.h"
#include "memory/Asset.h"


class Model;
class Skybox;
class Context;
class Object3D;
class PointLight;
class DirectionalLight;
class Renderer;
class ObjectSelection;
class PostProcessProjector;
class Shader;
class CoroutineSystem;
class Context;
class ObjectPicker;
class Transformation;
class GpuInstancingRenderer;
class Entity;
class IRenderer;
class Mesh;
class Transformation;
class InstanceBatch;
class SkyboxRenderer;
class ICamera;
class PhysicsSystem;
class ShadowSystem;
class UniformBufferObject;
class LightSystem;
class DeferredRenderer;
class FrameBufferObject;
class RenderBufferObject;
class Texture;
class SGE_Regsitry;
class Entity;
class RenderView;
class EventLayer;
class WireframeGrid;
namespace physx {
	class PxScene;
}
template<typename T> class ObjectHandler;

struct SceneData
{
	SerializedScene m_serializedScene;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_serializedScene);
	}
};

struct EngineAPI SceneCreateDescriptor : public ResourceCreateDescriptor
{
	SceneData data;

	ResourceWrapper<Resource> createResource() override;
};

struct EngineAPI SceneLoadDescriptor : public ResourceLoadDescriptor
{
	ResourceWrapper<Resource> loadResource() override;
};

// Resource
class EngineAPI Scene : public Resource
{
public:
	enum class RenderPhase
	{
		PRE_RENDER_BEGIN,
		PRE_RENDER_END,
		DRAW_QUEUE_PRE_RENDER,
		DRAW_QUEUE_POST_RENDER,
		POST_RENDER_BEGIN,
		POST_RENDER_END
	};

	using RenderCallback = std::function<void()>;
public:
	// -------------------- Methods -------------------- //
	Scene() = default;
	Scene(Context* context);

	static ResourceWrapper<Scene> load(const std::string& fileLocation, SceneLoadDescriptor desc = {});

	static ResourceWrapper<Scene> create();

	void addCoroutine(const std::function<bool(float)>& coroutine);

	uint32_t getID() const { return m_id; }

	RenderCallback* addRenderCallback(RenderPhase renderPhase, RenderCallback renderCallback);
	void removeRenderCallback(RenderCallback* callback);

	SGE_Regsitry& getRegistry();
	const SGE_Regsitry& getRegistry() const;

	Entity createEntity();
	Entity createEntity(const std::string& name);
	void removeEntity(const Entity& e);

	void displayWireframeMesh(Entity e);

	void setIBLData(ResourceWrapper<Texture> irradianceMap, ResourceWrapper<Texture> prefilterEnvMap);

	//int getRenderTarget() const;

	bool isSimulationActive() const;

	Entity getEntityByName(const std::string& name) const;

	physx::PxScene* getPhysicsScene() const;


	void addRenderView(const std::string& name, int x, int y, int w, int h, const Entity& e);
	//void setActiveRenderView(const std::string& name);
	void setRenderViewEnabled(const std::string& name, bool enabled);
	void setGameRenderViewEnabled(bool enabled);
	

	unsigned int getRenderViewFrameBufferID(const std::string& name) const;
	unsigned int getRenderViewTextureID(const std::string& name) const;

	unsigned int getGameRenderViewTextureID() const;
    unsigned int getGameRenderViewFrameBufferID() const;

    Entity getGameCamera() const;
    void setGameCamera(Entity e);

    glm::mat4 getGameCameraView() const;

    void onWindowResize(int w, int h);
	void init(Context* context);
private:
	// -------------------- Methods -------------------- //
	friend class Context;
	friend class SceneManager;
	friend class Archiver;
	void update(float deltaTime);
	inline void SetID(uint32_t id) { m_id = id; }
	void draw(float deltaTime);
	
	void startSimulation();
	void stopSimulation();
	
	void clear();
	void close();

	void bindScriptToLayer(entt::registry& reg, entt::entity entity);
	std::shared_ptr<RenderView> getRenderView(const std::string& name) const;

private:
	// -------------------- Attributes -------------------- //
	uint32_t m_id = 0;
	Context* m_context = nullptr;

	// Scene Services
	std::shared_ptr<ObjectSelection> m_objectSelection = nullptr;
	std::shared_ptr<PostProcessProjector> m_postProcessProjector = nullptr;
	std::shared_ptr<CoroutineSystem> m_coroutineManager = nullptr;
	std::shared_ptr<ObjectPicker> m_objectPicker = nullptr;
	std::shared_ptr<ShadowSystem> m_shadowSystem;
	std::shared_ptr<LightSystem> m_lightSystem;

	std::shared_ptr<SGE_Regsitry> m_registry;
	std::map<RenderPhase, std::vector<RenderCallback>> m_renderCallbacks;

	physx::PxScene* m_PhysicsScene = nullptr;

	bool m_isSimulationActive = false;

	ResourceWrapper<Shader> m_tempOutlineShader = nullptr;

	//CameraComponent* m_activeCamera = nullptr;

	glm::mat4 m_defaultPerspectiveProjection;
	glm::mat4 m_defaultUIProjection;

	std::shared_ptr<UniformBufferObject> m_uboTime;

	//std::shared_ptr<FrameBufferObject> m_renderTargetFBO;
	//std::shared_ptr<RenderBufferObject> m_renderTargetRBO;
	//Resource<Texture> m_renderTargetTexture;

	ResourceWrapper<Texture> m_irradianceMap;
	ResourceWrapper<Texture> m_prefilterEnvMap;
	ResourceWrapper<Texture> m_BRDFIntegrationLUT;
	ResourceWrapper<Shader> m_skyboxShader;
	ResourceWrapper<Shader> m_UIShader;

	//ResourceWrapper<MeshGroup> m_basicBox;

	//Entity m_primaryCamera = Entity::EmptyEntity;

	Entity m_quadUI;

	SerializedScene m_serializedScene;

	std::map<std::string, std::shared_ptr<RenderView>> m_renderViews;

	std::shared_ptr<EventLayer> gameEventLayer;

	std::shared_ptr<RenderView> m_highlightRenderView;

	ResourceWrapper<Shader> m_highlightMaskShader;
	ResourceWrapper<Shader> m_highlightEdgeDetectionShader;
	ResourceWrapper<Shader> m_highlightMergeShader;

	std::shared_ptr<WireframeGrid> m_wireframeGrid;

	ResourceWrapper<Shader> m_sampleComputeShader;
	
};

// Asset
class EngineAPI SceneAsset : public Asset
{
public:
	using ResourceType = Scene;

	using Asset::Asset;

	void serialize(nlohmann::json& j) const override;
	void deserialize(const nlohmann::json& j) override;
};