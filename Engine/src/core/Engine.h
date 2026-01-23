#pragma once

#include <memory>
#include <string>
#include <atomic>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include "core/Core.h"
#include "memory/UUID.h"
#include "core/GUILayer.h"


class Context;
class IRenderer;
class SkyboxRenderer;
class Window;
class ImguiHandler;
class ObjectSelection;
class PostProcessProjector;
class Application;
union SDL_Event;
class Input;
class EventSystem;
class ModelImporter;
class CacheSystem;
class ObjectManager;
class TimeManager;
class PhysicsSystem;
class RandomNumberGenerator;
class ShaderLoader;
class ResourceManager;
class Texture;
class Mesh;
class MeshCollection;
class ProjectManager;
class Material;
class Texture;
class CommonShaders;
class CommonTextures;
class ObjectPicker;
class Assets;
class Animation;
class AnimationLoader;
class Shader;
class EventLayerStack;
class BuiltInMeshes;
class BuiltInMaterials;
class Resource;
class SubSystem;
class DeferredRenderer;
class Renderer;
struct EngineConfig;
template<typename T>class ResourceWrapper;
template<typename T>class Factory;
template<typename T>class MemoryPool;

struct InitParams
{
    std::string projectDir;
    bool loadExistingProject = false;
    bool overwriteExisting = false;
    bool templateScene = false;
    bool startSimulationOnStartup = false;
    bool fullScreen = false;
    bool shipping = false;
};

class EngineAPI Engine
{
public:

    // -------------------- Methods -------------------- //
    static inline Engine* get()
    {
        if (instance == nullptr)
        {
            instance = new Engine();
        }
        return instance;
    }

    std::string getRootDir();

    Window* getWindow() const;
    Context* getContext() const;
    //ImguiHandler* getImguiHandler() const;
    Input* getInput() const;
    EventSystem* getEventSystem() const;
    CacheSystem* getMemoryManagementSystem() const;
    ObjectManager* getObjectManager() const;
    TimeManager* getTimeManager() const;
    PhysicsSystem* getPhysicsSystem() const;
    RandomNumberGenerator* getRandomSystem() const;
    ShaderLoader* getShaderLoader() const;
    ResourceManager* getResourceManager() const;
    const InitParams& getInitParams() const;
    EventLayerStack* getEventLayerStack() const;
    const EngineConfig& getConfig() const;
    MemoryPool<Resource>& getMemoryPool() const;
    DeferredRenderer& getDeferredRenderer() const;
    Renderer& getForwardRenderer() const;
    void addGUILayer(const std::shared_ptr<GUILayer>& GUILayer);
    
    void loadProject(const std::string& dirPath);
    void saveProject();
    std::string getProjectDirectory() const;

    ResourceWrapper<Material> getDefaultMaterial() const;

    void reloadEngineConfig();

    void pause();
    void resume();
    void stop();

    // Template function to register a subsystem
    template<typename T> 
    void registerSubSystem(const T* subSystem)
    {
        m_subSystems[typeid(T*)] = (SubSystem*)subSystem;
    }

    // Template function to get a registered subsystem
    template<typename T> 
    T* getSubSystem() const
    {
        auto it = m_subSystems.find(typeid(T*));
        if (it != m_subSystems.end())
        {
            return dynamic_cast<T*>(it->second);
        }
        else
        {
            throw std::exception("Subsystem not found");
        }
    }


    Engine(const Engine&) = delete;
    void operator=(const Engine&) = delete;

protected:
    // -------------------- Methods -------------------- //
    friend int sgeEntry(int, char*[]);
    template<typename T>friend class Factory;

#ifdef TESTBED
    friend class TestEngine;
#endif

    bool init(const InitParams& initParams);
    void update(float deltaTime);
    void draw(float deltaTime);
    void run(Application* app);
    void close();

    void SetContext(std::shared_ptr<Context> context);
    void SetWindow(std::shared_ptr<Window> window);
    void handleEvents(bool& quit);

    void createStartupScene(const std::shared_ptr<Context>& context, const InitParams& initParams);
protected:

    Engine();

    // -------------------- Attributes -------------------- //
    static Engine* instance;

    bool m_isInit = false;

    std::shared_ptr<Window> m_window;
    std::shared_ptr<Context> m_context;
    //std::shared_ptr<ImguiHandler> m_imguiHandler;
    std::vector<std::shared_ptr<GUILayer>> m_GUILayers;
    std::shared_ptr<Input> m_input;
    std::shared_ptr<EventSystem> m_eventSystem;
    std::shared_ptr<CacheSystem> m_memoryManagementSystem;
    std::shared_ptr<ObjectManager> m_objectManager;
    std::shared_ptr<TimeManager> m_timeManager;
    std::shared_ptr<PhysicsSystem> m_physicsSystem;
    std::shared_ptr<RandomNumberGenerator> m_randomSystem;
    std::shared_ptr<ShaderLoader> m_shaderLoader;
    std::shared_ptr<ResourceManager> m_resourceManager;
    std::shared_ptr<ProjectManager> m_projectManager;
    std::shared_ptr<EngineConfig> m_engineConfig;
    std::shared_ptr<MemoryPool<Resource>> m_memoryPool;
    std::shared_ptr<DeferredRenderer> m_deferredRenderer;
    std::shared_ptr<Renderer> m_forwardRenderer;

    InitParams m_initParams;

    std::map<std::type_index, SubSystem*> m_subSystems;

    std::atomic<bool> m_isPaused = false;
    std::atomic<bool> m_isStopped = false;

    std::string m_projectDirectory;

    std::shared_ptr<EventLayerStack> m_eventLayerStack;
};