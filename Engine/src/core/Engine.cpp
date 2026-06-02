#include "core/Engine.h"

#include "runtime/Context.h"
#include "render/Renderer.h"
#include "core/Window.h"
#include "camera/ICamera.h"
#include "runtime/Scene.h"
#include "ui/Input.h"
#include "core/EventSystem.h"
#include "core/GameEventSystem.h"
#include "geometry/ModelImporter.h"
#include "core/Logger.h"
#include "core/TraceLogger.h"
#include "core/CacheSystem.h"
#include "systems/TimeManager.h"
#include "physics/PhysicsSystem.h"
#include "core/Random.h"
#include "render/ShaderLoader.h"
#include "render/ShaderParser_tntmeijsImpl.h"
#include "memory/ResourceManager.h"
#include "memory/ProjectManager.h"
#include "serialize/ProjectAssetRegistry.h"
#include "geometry/ShapeFactory.h"
#include "render/Material.h"
#include "lights/DirectionalLight.h"
#include "systems/BuiltInAssetsLoader.h"
#include "memory/BuiltInAssets.h"
#include "systems/ObjectPicker.h"
#include "animation/AnimationLoader.h"
#include "memory/Assets.h"
#include "render/Graphics.h"
#include "core/System.h"
#include "render/RenderCommand.h"
#include "core/EventLayerStack.h"
#include "core/EngineConfig.h"
#include "core/EventParser.h"
#include "render/DeferredRenderer.h"
#include "memory/FrameAccessTable.h"

#include "systems/FoliageSystem.h"
#include "systems/WaterSystem.h"
#include "systems/VolumetricCloudsSystem.h"
#include "systems/VolumetricSystem.h"
#include "component/CameraComponent.h"
#include "component/MeshRendererComponent.h"
#include "component/PostProcessComponent.h"

#include "component/RenderableComponent.h"
#include "scripts/ScriptSystem.h"
#include "animation/AnimationSystem.h"
#include "core/GameLayer.h"
#include "systems/UniqueNameManager.h"
#include "memory/BuiltInResources.h"
#include "fileSystem/FileSystem.h"
#include "memory/MemoryPool.h"
#include "runtime/StartupSceneTemplate.h"
#include "core/ProjectSettings.h"

#include "core/Application.h"
#include "SDL2/SDL.h"

#include "GL/glew.h"

#include <filesystem>

#include "core/CoreEvents.h"
#include "imgui_impl_sdl.h"

// Singleton
Engine* Engine::instance = nullptr;

bool Engine::init(const InitParams& initParams)
{
    Logger::init("test.log");
    TraceLogger::init();

    if (m_isInit)
    {
        logError("Engine already started!");
        return false;
    }

    m_initParams = initParams;

    m_resourceManager = std::make_shared<ResourceManager>();
    if (!SGE_EXPORT_PACKAGE)
    {
        m_resourceManager->setRootDir(SGE_ROOT_DIR "/");
    }
    else
    {
        m_resourceManager->setRootDir("./");
    }

    m_engineConfig = std::make_shared<EngineConfig>(SGE_ROOT_DIR "/EngineConfig.json");

    if (initParams.tempDir)
    {
        m_projectDirectory = std::filesystem::temp_directory_path().string() + "/";
        std::filesystem::create_directories(m_projectDirectory + "Engine");
        std::filesystem::create_directories(m_projectDirectory + "Content");
    }
    else
    {
        m_projectDirectory = initParams.projectDir + "/";

        if (!std::filesystem::exists(m_projectDirectory) || !std::filesystem::is_directory(m_projectDirectory))
        {
            logError("Path does not exist or is not a directory");
            return false;
        }

        // If create new project, check if dir is empty
        if (!initParams.loadExistingProject)
        {
            if (!initParams.overwriteExisting)
            {
                // Iterate over the directory and check if there are any entries
                if (std::filesystem::directory_iterator(m_projectDirectory) != std::filesystem::directory_iterator{})
                {
                    logError("Directory is not empty, SGE requires an empty directory to start a new project.");
                    return false;
                }
            }
            else
            {
                // Overwrite existing files by deleting all files in the directory
                try {
                    for (const auto& entry : std::filesystem::directory_iterator(m_projectDirectory)) {
                        std::filesystem::remove_all(entry); // Remove file or directory
                    }
                }
                catch (const std::filesystem::filesystem_error& e) {
                    logError("Failed to clear the directory: " + std::string(e.what()));
                    return false;
                }
            }

            std::filesystem::create_directories(Engine::get()->getProjectDirectory());
            std::filesystem::create_directories(Engine::get()->getProjectDirectory() + "/Engine");
            std::filesystem::create_directories(Engine::get()->getProjectDirectory() + "/Content");
        }
    }

    auto filesystem = new FileSystem();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_3D);

    //glEnable(GL_DEBUG_OUTPUT);
    //glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_CULL_FACE);

    //glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_eventSystem = std::make_shared<EventSystem>();
    m_eventLayerStack = std::make_shared<EventLayerStack>();
    auto system = new System(); // TODO change to systemAnalytics
    m_timeManager = std::make_shared<TimeManager>();
    m_randomSystem = std::make_shared<RandomNumberGenerator>();

    Trace::setProjectRootFolder(m_projectDirectory);

    std::shared_ptr<GameLayer> gameEventLayer = std::make_shared<GameLayer>();
    gameEventLayer->setEnabled(false);
    m_eventSystem->pushLayer(gameEventLayer);

    m_memoryPool = std::make_shared<MemoryPool<Resource>>();

    m_projectManager = std::make_shared<ProjectManager>();

    m_window = std::make_shared<Window>();
    if (!m_window->init())
    {
        logError("Window init failed!");
        return false;
    }

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    m_input = std::make_shared<Input>();
    m_input->init();

    auto shaderParser = std::make_shared<ShaderParser_tntmeijs>();

    ShaderLoader::LoadParams lParams;
    lParams.extendShader = true;
    m_shaderLoader = std::make_shared<ShaderLoader>(shaderParser, lParams);

    for (auto& GUILayer : m_GUILayers)
    {
        if(!GUILayer->init())
        {
            logError("Imgui init failed!");
            return false;
        }

    }

    //m_imguiHandler = std::make_shared<ImguiHandler>();
    //if (!m_imguiHandler->init(m_window->GetWindow(), m_window->GetContext()))
    //{
    //    logError("Imgui init failed!");
    //    return false;
    //}

    auto frameAcessTable = new FrameAccessTable(5);
    auto builtInResources = new BuiltInResources();
    auto assets = new Assets();

    builtInResources->loadAllResources();

    auto modelImporter = new ModelImporter();
    auto animationLoader = new AnimationLoader();
    auto graphics = new Graphics();
    auto gameKeyboard = new GameKeyboard();
    auto gameMouse = new GameMouse();
    auto uniqueNameManager = new UniqueNameManager();
    auto gameEventSystem = new GameEventSystem();

    // Create or Load project asset registry
    std::shared_ptr<ProjectAssetRegistry> par;
    if (initParams.loadExistingProject)
    {
        par = ProjectAssetRegistry::parse(m_projectDirectory);
    }
    else
    {   
        par = ProjectAssetRegistry::create(m_projectDirectory);
    }

    m_memoryManagementSystem = std::make_shared<CacheSystem>(par);
    m_context = std::make_shared<Context>(par);
    BuiltInAssetsLoader::loadAssets();

    

    m_physicsSystem = std::make_shared<PhysicsSystem>();
    if (!m_physicsSystem->init())
    {
        logError("Physics System init failed!");
        return false;
    }

    auto foliageSystem = new FoliageSystem();
    if(!foliageSystem->init())
    {
        logError("Foliage System init failed!");
        return false;
    }

    auto waterSystem = new WaterSystem();
    auto volumetricSystem = new VolumetricSystem();
    if (!volumetricSystem->init())
    {
        logError("Volumetric System init failed!");
        return false;
    }
    auto volumetricCloudsSystem = new VolumetricCloudsSystem();

    auto scriptSystem = new ScriptSystem();
    scriptSystem->init();

    if (initParams.loadExistingProject)
    {
        loadProject(m_projectDirectory);
    }
    else
    {
        ProjectSettings::get().create(m_projectDirectory);
        auto startupScene = StartupSceneTemplate::createStartupScene("Scene_0");
        m_context->setActiveScene(startupScene.resource()->getID());
        saveProject();
    }

    
    m_deferredRenderer = std::make_shared<DeferredRenderer>();
    m_deferredRenderer->init();

    m_forwardRenderer = std::make_shared<Renderer>();
    m_forwardRenderer->init();

    m_indirectRenderer = std::make_shared<IndirectRenderer>();
    m_indirectRenderer->init();
    

    auto objectPicker = new ObjectPicker();
    if (!objectPicker->init())
    {
        logError("Object picker failed to init!");
        return false;
    }

    
   

    new AnimationSystem();

    if (initParams.startSimulationOnStartup)
    {
        m_context->startSimulation();
    }

    m_isInit = true;

    logInfo("SGE Initialized Successfully!");

    return true;
}

Window* Engine::getWindow() const
{
    return m_window.get();
}

Engine::Engine()
{}

std::string Engine::getRootDir()
{
    return m_resourceManager->getRootDir();
}

void Engine::SetWindow(std::shared_ptr<Window> window)
{
    m_window = window;
}

void Engine::SetContext(std::shared_ptr<Context> context)
{
    m_context = context;
}

Context* Engine::getContext() const
{
    return m_context.get();
}


void Engine::draw(float deltaTime)
{
    m_context->draw(deltaTime);
}


void Engine::update(float deltaTime)
{
    m_context->update(deltaTime);
}
#include "core/SGE_Exception.h"
void Engine::run(Application* app)
{
    //SGE_THROW("as");
    //Main loop flag
    bool quit = false;

    auto system = getSubSystem<System>();

    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;
    double deltaTime = 0;
    // Loop until window closed
    while (!quit)
    {
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();

        if (NOW == LAST) continue;

        deltaTime = ((NOW - LAST) / (double)SDL_GetPerformanceFrequency());

        system->tick(deltaTime);

        //Handle events on queue
        handleEvents(quit);

        m_window->update();

        if (quit)
            return;

        update(deltaTime);
        draw(deltaTime);
        app->update(deltaTime);

        RenderCommand::copyFrameBufferData(m_context->getActiveScene()->getGameRenderViewFrameBufferID(), 
            0, 
            RenderCommand::BufferBit::DEPTH_BUFFER_BIT | RenderCommand::BufferBit::COLOR_BUFFER_BIT);

        //glBindFramebuffer(GL_READ_FRAMEBUFFER, m_context->getActiveScene()->getRenderTarget());
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        //glBlitFramebuffer(0, 0, m_window->getWidth(), m_window->getHeight(), 0, 0, m_window->getWidth(), m_window->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "GUI render pass");
        for (auto& GUILayer : m_GUILayers)
        {
            GUILayer->render();

        }
        glPopDebugGroup();

        m_window->SwapBuffer();

        // Advance frame for the access table to evict old resources
        getSubSystem<FrameAccessTable>()->nextFrame();

        system->reset();

    }
}

void Engine::stop()
{
    logTrace(__FUNCTION__);

    QuitAppEvent e;
    m_eventSystem->pushEvent(e);
}

void Engine::close()
{
    logTrace(__FUNCTION__);

    m_context->close();

    for (auto& GUILayer : m_GUILayers)
    {
        GUILayer->close();

    }

    m_window->close();

    TraceLogger::shutdown();

    m_isInit = false;

    // TODO fix

    //delete instance;
    //instance = nullptr;
}

//ImguiHandler* Engine::getImguiHandler() const
//{
//    return m_imguiHandler.get();
//}

Input* Engine::getInput() const
{
    return m_input.get();
}

EventSystem* Engine::getEventSystem() const
{
    return m_eventSystem.get();
}

CacheSystem* Engine::getMemoryManagementSystem() const
{
    return m_memoryManagementSystem.get();
}

ObjectManager* Engine::getObjectManager() const
{
    return m_objectManager.get();
}

TimeManager* Engine::getTimeManager() const
{
    return m_timeManager.get();
}

PhysicsSystem* Engine::getPhysicsSystem() const
{
    return m_physicsSystem.get();
}

RandomNumberGenerator* Engine::getRandomSystem() const
{
    return m_randomSystem.get();
}

ShaderLoader* Engine::getShaderLoader() const
{
    return m_shaderLoader.get();
}

ResourceManager* Engine::getResourceManager() const
{
    return m_resourceManager.get();
}

const InitParams& Engine::getInitParams() const
{
    return m_initParams;
}

EventLayerStack* Engine::getEventLayerStack() const
{
    return m_eventLayerStack.get();
}

const EngineConfig& Engine::getConfig() const
{
    return *m_engineConfig.get();
}

MemoryPool<Resource>& Engine::getMemoryPool() const
{
    return *m_memoryPool.get();
}

DeferredRenderer& Engine::getDeferredRenderer() const
{
    return *m_deferredRenderer.get();
}

Renderer& Engine::getForwardRenderer() const
{
    return *m_forwardRenderer.get();
}

IndirectRenderer& Engine::getIndirectRenderer() const
{
    return *m_indirectRenderer.get();
}

void Engine::addGUILayer(const std::shared_ptr<GUILayer>& GUILayer)
{
    m_GUILayers.push_back(GUILayer);
}

void Engine::setImGuiContext(void* context)
{
    ImGui::SetCurrentContext((ImGuiContext*)context);
}

void Engine::loadProject(const std::string& dirPath)
{
    m_projectDirectory = dirPath;
    
    m_projectManager->loadProject(getContext()->getProjectAssetRegistry()->getFilepath(), m_context);
}

void Engine::saveProject()
{
    m_projectManager->saveProject();
}

std::string Engine::getProjectDirectory() const
{
    return m_projectDirectory;
}

MaterialResourceRef Engine::getDefaultMaterial() const
{
    return BuiltInAssets::getByName<MaterialAsset>(SGE_MATERIAL_DEFAULT).resource();
}

void Engine::reloadEngineConfig()
{
    m_engineConfig->loadConfig();
}

void Engine::pause()
{
}

void Engine::handleEvents(bool& quit)
{
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_WINDOWEVENT &&
            (e.window.event == SDL_WINDOWEVENT_RESIZED || e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED))
        {
            m_window->resize(e.window.data1, e.window.data2);
            if (m_context && m_context->getActiveScene())
            {
                m_context->getActiveScene()->onWindowResize(e.window.data1, e.window.data2);
            }
        }

        if (e.type == SDL_QUIT)
        {
            quit = true;
        }

        if (e.type == SDL_DROPFILE)
        {
            logInfo("File dropped: {}", e.drop.file);
            SDL_free(e.drop.file);
        }

        if (ImGui::GetCurrentContext())
        {
            ImGui_ImplSDL2_ProcessEvent(&e);
        }

        //getSubSystem<RawEventDispatcher>()->dispatch(e);

        auto engineEvent = EventParser::parseSDLEvent(e);

        if (engineEvent)
            m_eventSystem->dispatch(*engineEvent);
    }

    // Handle custom events
    std::unique_ptr<Event> customEvent;
    while ((customEvent = m_eventSystem->pollEvent()) != nullptr)
    {
        if (customEvent->type() == EventType::QuitApp)
        {
            quit = true;
        }

        m_eventSystem->dispatch(*customEvent);
    }
}

void Engine::resume()
{
}
