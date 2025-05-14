#include "Engine.h"

#include "Context.h"
#include "Renderer.h"
#include "SkyboxRenderer.h"
#include "Window.h"
#include "ImguiHandler.h"
#include "PostProcessProjector.h"
#include "ICamera.h"
#include "Scene.h"
#include "Skybox.h"
#include "Input.h"
#include "EventSystem.h"
#include "ModelImporter.h"
#include "Logger.h"
#include "CacheSystem.h"
#include "TimeManager.h"
#include "PhysicsSystem.h"
#include "Random.h"
#include "ShadowSystem.h"
#include "ShaderLoader.h"
#include "ShaderParser_tntmeijsImpl.h"
#include "ResourceManager.h"
#include "ProjectManager.h"
#include "ProjectAssetRegistry.h"
#include "ShapeFactory.h"
#include "Material.h"
#include "DirectionalLight.h"
#include "EditorCamera.h"
#include "CommonShaders.h"
#include "CommonTextures.h"
#include "ObjectPicker.h"
#include "AnimationLoader.h"
#include "Assets.h"
#include "Graphics.h"
#include "System.h"
#include "RenderCommand.h"
#include "EventLayerStack.h"

#include "Application.h"
#include "SDL2/SDL.h"

#include "GL/glew.h"

#include <filesystem>

// Singleton
Engine* Engine::instance = nullptr;

bool Engine::init(const InitParams& initParams)
{
    if (m_isInit)
    {
        logError("Engine already started!");
        return false;
    }

    m_initParams = initParams;

    m_projectDirectory = initParams.projectDir;

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

    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_3D);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_eventSystem = std::make_shared<EventSystem>();
    m_eventLayerStack = std::make_shared<EventLayerStack>();

    m_memoryPoolTexture = std::make_shared<MemoryPool<Texture>>();
    m_memoryPoolMeshCollection = std::make_shared<MemoryPool<MeshCollection>>();
    m_memoryPoolAnimation = std::make_shared<MemoryPool<Animation>>();
    m_memoryPoolShader = std::make_shared<MemoryPool<Shader>>();

    m_resourceManager = std::make_shared<ResourceManager>();

    m_projectManager = std::make_shared<ProjectManager>();

    if (!SGE_EXPORT_PACKAGE)
    {
        auto found = false;
        try
        {
            size_t len = 0;
            char* sgeRoot = nullptr;
            errno_t err = _dupenv_s(&sgeRoot, &len, "SGE");
            if (err == 0 && sgeRoot)
            {
                found = true;
                m_resourceManager->setRootDir(std::string(sgeRoot) + "/Engine/");
            }
            free(sgeRoot);
        }
        catch (std::exception e)
        {
            logError(e.what());
        }

        if (!found)
        {
            m_resourceManager->setRootDir("./");
        }
        
    }
    else
    {
        m_resourceManager->setRootDir("./");
    }

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

    //auto secondScene = std::make_shared<Scene>();
    //secondScene->setPostProcess(true);
    //m_context->addScene(secondScene);

    m_imguiHandler = std::make_shared<ImguiHandler>();
    if (!m_imguiHandler->init(m_window->GetWindow(), m_window->GetContext()))
    {
        logError("Imgui init failed!");
        return false;
    }

    auto modelImporter = new ModelImporter();
    auto animationLoader = new AnimationLoader();
    auto graphics = new Graphics();
    auto system = new System();
    m_assets = std::make_shared<Assets>();

    m_timeManager = std::make_shared<TimeManager>();
    m_physicsSystem = std::make_shared<PhysicsSystem>();
    if (!m_physicsSystem->init())
    {
        logError("Physics System init failed!");
        return false;
    }

    m_memoryManagementSystem = std::make_shared<CacheSystem>();

    m_randomSystem = std::make_shared<RandomNumberGenerator>();

    if (initParams.loadExistingProject)
    {
        loadProject(m_projectDirectory);
    }
    else
    {
        // Create a new Project       
        auto& par = ProjectAssetRegistry::create(initParams.projectDir);;
        m_context = std::make_shared<Context>(par);

        m_commonTextures = std::shared_ptr<CommonTextures>(CommonTextures::create());
        m_commonShaders = std::make_shared<CommonShaders>();
        m_defaultMaterial = std::make_shared<Material>();
        
        createStartupScene(m_context, initParams);

        saveProject();
    }

   

    auto objectPicker = new ObjectPicker();
    if (!objectPicker->init())
    {
        logError("Object picker failed to init!");
        return false;
    }

    if (initParams.startSimulationOnStartup)
    {
        m_context->getActiveScene()->startSimulation();
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

Engine* Engine::get()
{
    if (instance == nullptr)
    {
        instance = new Engine();
    }
    return instance;
}

std::string Engine::getRootDir()
{
    return m_resourceManager->getRootDir();
}

void Engine::SetWindow(std::shared_ptr<Window> window)
{
    m_window = window;
}

IRenderer* Engine::getRenderer() const
{
    return m_context->getActiveScene()->getRenderer().get();
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
#include "SGE_Exception.h"
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

        system->setDeltaTime(deltaTime);

        //Handle events on queue
        handleEvents(quit);

        m_window->update();

        if (quit)
            return;

        update(deltaTime);
        draw(deltaTime);
        app->update(deltaTime);

        RenderCommand::copyFrameBufferData(m_context->getActiveScene()->getRenderTargetFrameBufferID(0), 
            0, 
            RenderCommand::BufferBit::DEPTH_BUFFER_BIT | RenderCommand::BufferBit::COLOR_BUFFER_BIT);

        //glBindFramebuffer(GL_READ_FRAMEBUFFER, m_context->getActiveScene()->getRenderTarget());
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        //glBlitFramebuffer(0, 0, m_window->getWidth(), m_window->getHeight(), 0, 0, m_window->getWidth(), m_window->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);

        m_imguiHandler->render();

        m_window->SwapBuffer();

        system->reset();

    }
}

void Engine::stop()
{
    logTrace(__FUNCTION__);

    SDL_Event e;
    e.type = SDL_QUIT;
    m_eventSystem->pushEvent(e);
}

void Engine::close()
{
    logTrace(__FUNCTION__);

    m_context->close();

    m_imguiHandler->close();

    m_window->close();

    m_isInit = false;

    // TODO fix

    //delete instance;
    //instance = nullptr;
}

ImguiHandler* Engine::getImguiHandler() const
{
    return m_imguiHandler.get();
}

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

CommonShaders* Engine::getCommonShaders() const
{
    return m_commonShaders.get();
}

CommonTextures* Engine::getCommonTextures() const
{
    return m_commonTextures.get();
}

const InitParams& Engine::getInitParams() const
{
    return m_initParams;
}

EventLayerStack* Engine::getEventLayerStack() const
{
    return m_eventLayerStack.get();
}

void Engine::loadProject(const std::string& dirPath)
{
    m_projectDirectory = dirPath;


    auto& par = ProjectAssetRegistry::parse(dirPath);
    auto& filePath = par->getFilepath();
    m_context = std::make_shared<Context>(par);

    m_memoryManagementSystem = std::make_shared<CacheSystem>(par->getAssociations());
    m_commonTextures = std::shared_ptr<CommonTextures>(CommonTextures::load());
    m_defaultMaterial = std::make_shared<Material>();

    m_projectManager->loadProject(filePath, m_context);
}

void Engine::saveProject()
{
    m_projectManager->saveProject();
}

std::string Engine::getProjectDirectory() const
{
    return m_projectDirectory;
}

std::shared_ptr<Material> Engine::getDefaultMaterial() const
{
    return m_defaultMaterial;
}

void Engine::pause()
{
}

void Engine::handleEvents(bool& quit)
{
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0)
    {



        //m_imguiHandler->proccessEvents(e);

        //User requests quit
        if (e.type == SDL_QUIT)
        {
            quit = true;
        }


        m_eventSystem->dispatch(e);
    }
}

void Engine::createStartupScene(const std::shared_ptr<Context>& context, const InitParams& initParams)
{
    auto startupScene = std::make_shared<Scene>(m_context.get());

    m_context->addScene(startupScene);
    m_context->setActiveScene(startupScene->getID());

    // Add default dir light
    auto dLight = startupScene->createEntity("Directional light");
    dLight.addComponent<DirectionalLight>();
    dLight.getComponent<Transformation>().setLocalRotation(glm::vec3(0, -1, 0));

    auto mainCamera = startupScene->createEntity("Main Camera");
    mainCamera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)4 / 3, 0.1f, 1000.0f));
    startupScene->setPrimaryCamera(mainCamera);
    mainCamera.getComponent<Transformation>().setLocalPosition({10,10,10});
    mainCamera.getComponent<CameraComponent>().center = {0,0,0};
    mainCamera.getComponent<CameraComponent>().up = {0,1,0};

    auto modelInfo = getSubSystem<ModelImporter>()->import(SGE_ROOT_DIR + "Resources/Engine/Meshes/camera.obj");
    mainCamera.addComponent<MeshComponent>().mesh = modelInfo.mesh;
    mainCamera.addComponent<RenderableComponent>();

    auto& materialComponent = mainCamera.addComponent<MaterialComponent>();
    for (auto& [idx, m] : modelInfo.materials)
    {
        materialComponent.setMaterial(idx, m);
    }

    if (initParams.templateScene)
    {
        //Skybox::CreateSkyboxFromEquirectangularMap( "C:/dev/repos/LearnOpenGL/resources/textures/hdr/newport_loft.hdr", context->getActiveScene().get());
        //Skybox::CreateSkyboxFromCubemap({ SGE_ROOT_DIR + "Resources/Engine/Textures/Skybox/right.jpg",
        //SGE_ROOT_DIR + "Resources/Engine/Textures/Skybox/left.jpg",
        //SGE_ROOT_DIR + "Resources/Engine/Textures/Skybox/top.jpg",
        //SGE_ROOT_DIR + "Resources/Engine/Textures/Skybox/bottom.jpg",
        //SGE_ROOT_DIR + "Resources/Engine/Textures/Skybox/front.jpg",
        //SGE_ROOT_DIR + "Resources/Engine/Textures/Skybox/back.jpg" }, context->getActiveScene().get());

        // todo revert
        //{
        //    auto ground = ShapeFactory::createBox(&context->getActiveScene()->getRegistry());
        //    auto& groundTransfrom = ground.getComponent<Transformation>();
        //    groundTransfrom.setLocalScale({ 50, .5f, 50 });
        //    auto& mat = ground.addComponent<MaterialComponent>();
        //    auto tex = Engine::get()->getSubSystem<Assets>()->importTexture2D(SGE_ROOT_DIR + "Resources/Engine/Textures/floor.jpg");
        //    mat.begin()->get()->setTexture(Texture::Type::Albedo, tex);
        //    auto& rb = ground.addComponent<RigidBodyComponent>(RigidbodyType::Static, 1.f);
        //    auto& collisionBox = ground.addComponent<CollisionBoxComponent>(.5f);
        //}

        {
            //editorCamera->lookAt(0, 5, 0);
            //editorCamera->setPosition(25, 225, 35);
        }

        ShapeFactory::createSphere(&context->getActiveScene()->getRegistry());
    }
}

void Engine::resume()
{
}
