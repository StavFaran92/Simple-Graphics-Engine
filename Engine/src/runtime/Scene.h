#pragma once

#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <map>
#include <functional>

#include "core/Core.h"
#include "core/Subscriber.h"

#include "glm/glm.hpp"
#include "runtime/Entity.h"
#include "serialize/Archiver.h"
#include "memory/AssetDescriptors.h"
#include "memory/Asset.h"
#include "memory/AssetAliases.h"
#include "memory/ResourceRef.h"


class Model;
class Skybox;
class Context;
class Object3D;
class PointLight;
class DirectionalLight;
class ForwardRenderer;
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

	std::stringstream registryStream;
};

struct EngineAPI SceneCreateDescriptor : public ResourceBuildDescriptor
{
	SceneData data;
};

struct EngineAPI SceneLoadDescriptor : public ResourceLoadDescriptor
{
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

	static SceneResourceRef load(const std::string& fileLocation, SceneLoadDescriptor desc = {});

	static SceneResourceRef create();

	void onActivate();
	void onDeactivate();

	void addCoroutine(const std::function<bool(float)>& coroutine);

	uint32_t getID() const { return m_id; }

	RenderCallback* addRenderCallback(RenderPhase renderPhase, RenderCallback renderCallback);
	void removeRenderCallback(RenderCallback* callback);

	SGE_Regsitry& getRegistry();
	const SGE_Regsitry& getRegistry() const;

	Entity createEntity();
	Entity createEntity(const std::string& name);
	void removeEntity(const Entity& e);

	void setIBLData(TextureResourceRef irradianceMap, TextureResourceRef prefilterEnvMap);

	//int getRenderTarget() const;

	bool isSimulationActive() const;
	bool isSimulationPaused() const;

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
	void init(Context* context, ResourceID rid);

	void makeDirty();
	bool isSerializationDirty() const;

	bool isReady() const;
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
	void pauseSimulation();
	void resumeSimulation();
	
	void clear();
	void close();

	void bindScriptToLayer(entt::registry& reg, entt::entity entity);
	std::shared_ptr<RenderView> getRenderView(const std::string& name) const;

	void preloadSceneResources();

	ResourceID m_rid = 0;

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

	enum class SimState
	{
		STOPPED = 0,
		ACTIVE,
		PAUSED

	};

	SimState m_simulationState = SimState::STOPPED;

	ShaderResourceRef m_tempOutlineShader = nullptr;

	//CameraComponent* m_activeCamera = nullptr;

	glm::mat4 m_defaultPerspectiveProjection;
	glm::mat4 m_defaultUIProjection;

	std::shared_ptr<UniformBufferObject> m_uboTime;

	//std::shared_ptr<FrameBufferObject> m_renderTargetFBO;
	//std::shared_ptr<RenderBufferObject> m_renderTargetRBO;
	//Resource<Texture> m_renderTargetTexture;

	TextureResourceRef m_irradianceMap;
	TextureResourceRef m_prefilterEnvMap;
	TextureResourceRef m_BRDFIntegrationLUT;
	ShaderResourceRef m_skyboxShader;
	ShaderResourceRef m_UIShader;

	//ResourceWrapper<MeshGroup> m_basicBox;

	//Entity m_primaryCamera = Entity::EmptyEntity;

	SerializedScene m_serializedScene;

	std::map<std::string, std::shared_ptr<RenderView>> m_renderViews;

	std::shared_ptr<EventLayer> gameEventLayer;

	std::shared_ptr<RenderView> m_highlightRenderView;

	ShaderResourceRef m_highlightMaskShader;
	ShaderResourceRef m_highlightEdgeDetectionShader;
	ShaderResourceRef m_highlightMergeShader;

	ShaderResourceRef m_debugVisualizeShader;

	std::shared_ptr<WireframeGrid> m_wireframeGrid;

	ShaderResourceRef m_sampleComputeShader;


	struct CachedResource
	{
		ResourceRef<Resource> resource;
		uint64_t version = 0;
	};
	std::unordered_map<UUID, CachedResource> m_cachedResources;;

	bool m_isDirty = false;

	bool m_isReady = false; // False means scene is still loading
	
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
