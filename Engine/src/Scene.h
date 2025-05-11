#pragma once

#include <vector>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <map>
#include <functional>

#include "Core.h"

#include "glm/glm.hpp"
#include "IRenderer.h"
#include "Entity.h"
#include "Archiver.h"


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
class TextureHandler;
class SGE_Regsitry;
class Entity;
class RenderView;
struct RigidBodyComponent;
class GameLayer;
namespace physx {
	class PxScene;
}
template<typename T> class ObjectHandler;

class EngineAPI Scene
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
	Scene(Context* context);

	void setPostProcess(bool value);
	bool setPostProcessShader(Resource<Shader> shader);

	void addCoroutine(const std::function<bool(float)>& coroutine);
	void removeCoroutine(std::function<bool(float)> coroutine);

	std::shared_ptr<IRenderer> getRenderer() const;

	uint32_t getID() const { return m_id; }

	RenderCallback* addRenderCallback(RenderPhase renderPhase, RenderCallback renderCallback);
	void removeRenderCallback(RenderCallback* callback);

	SGE_Regsitry& getRegistry();
	const SGE_Regsitry& getRegistry() const;

	Entity createEntity();
	Entity createEntity(const std::string& name);
	void removeEntity(const Entity& e);

	glm::mat4 getActiveCameraView() const;
	
	void displayWireframeMesh(Entity e);

	void setIBLData(Resource<Texture> irradianceMap, Resource<Texture> prefilterEnvMap);

	//int getRenderTarget() const;

	glm::mat4 getProjection() const;

	Entity getActiveCamera() const;
	void setPrimaryCamera(Entity e);

	void startSimulation();
	void stopSimulation();
	bool isSimulationActive() const;

	Entity getEntityByName(const std::string& name) const;

	physx::PxScene* getPhysicsScene() const;

	unsigned int addRenderView(int x, int y, int w, int h, const Entity& e);

	unsigned int getRenderTargetFrameBufferID(unsigned int) const;
	unsigned int getRenderTargetTextureID(unsigned int) const;


	//void setPrimaryCamera(ICamera* camera);

private:
	// -------------------- Methods -------------------- //
	friend class Context;
	friend class Archiver;
	void update(float deltaTime);
	inline void SetID(uint32_t id) { m_id = id; }
	void draw(float deltaTime);
	
	void init(Context* context);
	void clear();
	void close();

private:
	// -------------------- Attributes -------------------- //
	uint32_t m_id = 0;
	Context* m_context = nullptr;

	// Renderers
	std::shared_ptr<DeferredRenderer> m_deferredRenderer = nullptr;
	std::shared_ptr<Renderer> m_forwardRenderer = nullptr;
	//std::shared_ptr<GpuInstancingRenderer> m_gpuInstancingRenderer = nullptr;
	//std::shared_ptr<SkyboxRenderer> m_skyboxRenderer = nullptr;

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

	Resource<Shader> m_tempOutlineShader = nullptr;

	//CameraComponent* m_activeCamera = nullptr;

	glm::mat4 m_defaultPerspectiveProjection;
	glm::mat4 m_defaultUIProjection;

	std::shared_ptr<UniformBufferObject> m_uboTime;

	//std::shared_ptr<FrameBufferObject> m_renderTargetFBO;
	//std::shared_ptr<RenderBufferObject> m_renderTargetRBO;
	//Resource<Texture> m_renderTargetTexture;

	Resource<Texture> m_irradianceMap;
	Resource<Texture> m_prefilterEnvMap;
	Resource<Texture> m_BRDFIntegrationLUT;
	Resource<Shader> m_skyboxShader;
	Resource<Shader> m_UIShader;
	Resource<Shader> m_terrainShader;

	Resource<MeshCollection> m_basicBox;

	//Entity m_primaryCamera = Entity::EmptyEntity;

	Entity m_quadUI;

	SerializedScene m_serializedScene;

	std::vector<std::shared_ptr<RenderView>> m_renderViews;

	std::shared_ptr<GameLayer> gameEventLayer;
};
