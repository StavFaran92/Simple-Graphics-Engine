#include "EntryPoint.h"
#include "sge.h"

#include "imgui_internal.h"

#include "tinyfiledialogs.h"

#include "NativeScriptsLoader.h"
#include "UIEventLayer.h"
#include "UIEditorToolsEventLayer.h"
#include "UINavigationLayer.h"
#include "UIEntitySelectionLayer.h"

#include "EditorCamera.h"
#include "EditorState.h"
#include "DialogManager.h"

#include "core/Logger.h"
#include <vector>
#include <mutex>

#include "Common.h"
#include "AssetViewWindow.h"

#include "EntityState.h"
#include "InspectorWindow.h"
#include "AssetInspectorWindow.h"
#include "AnimationGraphWindow.h"
#include "AnimationViewerWindow.h"
#include "SceneHierarchyWindow.h"
#include "SceneViewWindow.h"
#include "menus/CreateMenu.h"


#include "ImguiHandler.h"
#include "GUIMenu.h"

extern bool g_testRay;

static const std::string SGE_EDITOR_APP_ROOT = "../../EditorApp/Resources";
std::shared_ptr<EventLayer> uiLayer = std::make_shared<UIEventLayer>();
std::shared_ptr<EventLayer> uiEditorToolsLayer = std::make_shared<UIEditorToolsEventLayer>();
std::shared_ptr<EventLayer> uiNavigationLayer = std::make_shared<UINavigationLayer>();
std::shared_ptr<EventLayer> uiEntitySelectionLayer = std::make_shared<UIEntitySelectionLayer>();

namespace fs = std::filesystem;

struct Message
{
	spdlog::level::level_enum level;
	std::string msg;
};

inline ImVec4 LogLevelToColor(spdlog::level::level_enum level)
{
	switch (level)
	{
	case spdlog::level::trace:
		return ImVec4(0.7f, 0.7f, 0.7f, 1.0f); // light gray
	case spdlog::level::debug:
		return ImVec4(0.3f, 0.8f, 0.3f, 1.0f); // green
	case spdlog::level::info:
		return ImVec4(0.2f, 0.6f, 1.0f, 1.0f); // blue
	case spdlog::level::warn:
		return ImVec4(1.0f, 0.8f, 0.2f, 1.0f); // yellow/orange
	case spdlog::level::err:
		return ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // red
	case spdlog::level::critical:
		return ImVec4(0.9f, 0.1f, 0.1f, 1.0f); // dark red
	case spdlog::level::off:
		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // white (disabled)
	default:
		return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // fallback white
	}
}

// Console log storage
static std::vector<Message> g_consoleLog;
static std::mutex g_consoleMutex;
static bool g_scrollConsole = false;

static void appendConsoleLog(spdlog::level::level_enum level, const std::string& msg)
{
	std::lock_guard<std::mutex> lock(g_consoleMutex);
	g_consoleLog.push_back({ level, msg });
	g_scrollConsole = true;
}

class ConsoleLoggerRegister
{
public:
	ConsoleLoggerRegister()
	{
		Logger::setCallback(appendConsoleLog);
	}
};

static ConsoleLoggerRegister clr;
Entity g_primaryCamera;
Entity g_editorCamera;

uint32_t g_previewWindowID = 0;

static void stopSimulation()
{
	EditorState::Instance().startButtonPressed = false; // Toggle the state
	Engine::get()->getContext()->stopSimulation();

	//Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(g_editorCamera);
	Engine::get()->getContext()->getActiveScene()->setGameRenderViewEnabled(false);
	Engine::get()->getContext()->getActiveScene()->setRenderViewEnabled("Editor View", true);
	uiLayer->setEnabled(true);
	static_cast<EditorCamera*>(g_editorCamera.getComponent<NativeScriptComponent>().script.get())->unlock(); //TODO this should be in camera event
}

static void startsimulation()
{
	EditorState::Instance().startButtonPressed = true; // Toggle the state
	Engine::get()->getContext()->startSimulation();

	//Engine::get()->getContext()->getActiveScene()->setPrimaryCamera(g_primaryCamera);
	uiLayer->setEnabled(false);
	Engine::get()->getContext()->getActiveScene()->setGameRenderViewEnabled(true);
	Engine::get()->getContext()->getActiveScene()->setRenderViewEnabled("Editor View", false);
	state.selectEntity(Entity::EmptyEntity);
	static_cast<EditorCamera*>(g_editorCamera.getComponent<NativeScriptComponent>().script.get())->lock(); //TODO this should be in camera event

}

void RenderSimulationControlView()
{
	ImGui::Begin("Simulation Controls", nullptr, windowFlags | ImGuiWindowFlags_NoResize);

	float windowWidth = ImGui::GetContentRegionAvail().x;
	ImGui::SetCursorPosX((windowWidth - 100) * 0.5f);

	// Draw the button based on the current state
	if (EditorState::Instance().startButtonPressed)
	{
		if (ImGui::Button("STOP", ImVec2(70, 0)))
		{
			// Handle stop button click
			stopSimulation();
		}
	}
	else
	{
		if (ImGui::Button("START", ImVec2(70, 0)))
		{
			// Handle start button click
			startsimulation();
		}
	}

	ImGui::End(); // End the window
}                                           

void LightCreatorWindow()
{
	if (EditorState::Instance().showLightCreatorWindow)
	{
		ImGui::SetNextWindowSize({ 400, 300 }, ImGuiCond_Appearing);
		ImGui::Begin("Light Creator");

		static LightType lightType = LightType::DirectionalLight;
		static float ambientIntensity = 0.2f;
		static float diffuseIntensity = 0.5f;
		static glm::vec3 color(1.f, 1.f, 1.f);
		static glm::vec3 dir(0.f, 0.f, 0.f);
		static glm::vec3 pos(0.f, 0.f, 0.f);
		static Attenuation attenuation;

		// Light Type
		ImGui::RadioButton("Directional Light", (int*)&lightType, 0);
		ImGui::RadioButton("Point Light", (int*)&lightType, 1);

		ImGui::ColorEdit3("Color", (float*)&color);
		ImGui::SliderFloat("Ambient intensity", &ambientIntensity, 0.0f, 1.0f);
		ImGui::SliderFloat("Diffuse intensity", &diffuseIntensity, 0.0f, 1.0f);

		if (lightType == LightType::DirectionalLight)
		{
			ImGui::InputFloat3("Direction", (float*)&dir);
		}
		if (lightType == LightType::PointLight)
		{
			ImGui::InputFloat3("Position", (float*)&pos);
			ImGui::LabelText("", "Attenuation");
			ImGui::SliderFloat("constant", (float*)&attenuation.constant, 0.f, 1.f);
			ImGui::SliderFloat("linear", (float*)&attenuation.linear, 0.f, 1.f);
			ImGui::SliderFloat("quadratic", (float*)&attenuation.quadratic, 0.f, 1.f);
		}

		if (ImGui::Button("Ok"))
		{
			if (lightType == LightType::DirectionalLight)
			{
				auto e = Engine::get()->getContext()->getActiveScene()->createEntity();
				e.addComponent<DirectionalLight>(color, dir, ambientIntensity, diffuseIntensity);

			}
			else if (lightType == LightType::PointLight)
			{
				auto e = Engine::get()->getContext()->getActiveScene()->createEntity();
				e.addComponent<PointLight>(color, ambientIntensity, diffuseIntensity, attenuation);
			}

			EditorState::Instance().showLightCreatorWindow = false;

			updateScene();

			//logInfo("Added light successfully.");
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			EditorState::Instance().showLightCreatorWindow = false;
		}

		ImGui::End();
	}

}

void RenderConsoleWindow()
{
        ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus |
											ImGuiWindowFlags_NoCollapse |
											ImGuiWindowFlags_NoFocusOnAppearing |
											ImGuiWindowFlags_NoTitleBar |
											ImGuiWindowFlags_NoMove);

        std::lock_guard<std::mutex> lock(g_consoleMutex);
        for (const auto& line : g_consoleLog)
        {
            ImGui::TextColored(LogLevelToColor(line.level), line.msg.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() || g_scrollConsole)
        {
            ImGui::SetScrollHereY(1.0f);
            g_scrollConsole = false;
        }

        ImGui::End();
}

void DisplayDebugInfoWindow()
{
	//if (displayDebugInfoWindow)
	ImVec2 windowSize(400.f, 400.f);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
	ImGui::Begin("Debug Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	// --- Basic Debug Info ---
	auto fps = Engine::get()->getSubSystem<System>()->getFPS();
	ImGui::Text("FPS: %.1f", fps);

	auto deltaTime = Engine::get()->getSubSystem<System>()->getDeltaTime() * 1000;
	ImGui::Text("Delta time: %.1f ms", deltaTime);

	auto triangleCount = Engine::get()->getSubSystem<System>()->getTriangleCount();
	ImGui::Text("Triangle count: %u", triangleCount);

	ImGui::Separator();

	// --- Texture List ---
	const auto& debugTextures = DebugHelper::getInstance().getDebugTextures();

	ImGui::Text("Debug Textures:");
	for (const auto& [name, textureResourceWrapper] : debugTextures)
	{
		// Show image button
		ImGui::BeginGroup();
		ImGui::PushID(name.c_str());

		if (ImGui::CollapsingHeader(name.c_str()))
		{
			ImTextureID texID = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(textureResourceWrapper.get()->getID()));
			if (ImGui::ImageButton(texID, ImVec2(100, 100)))
			{
				EditorState::Instance().selectedTextureName = name;
				EditorState::Instance().showTextureDisplayWindow = true;
				ImGui::OpenPopup("Texture Preview");
			}
		}


		ImGui::PopID();
		ImGui::EndGroup();
	}

	if (ImGui::Button("Reload Shaders"))
	{
		Engine::get()->getSubSystem<Graphics>()->reloadShaders();
	}

	if (ImGui::Button("Trigger mock event"))
	{
		Event e;
		Engine::get()->getEventSystem()->dispatch(e);
	}

	ImGui::End();
}


class GUI_Helper : public GuiMenu {
	// Inherited via GuiMenu
	virtual void display() override {

	RenderCommand::clear();

	// Get the total screen size
	int screenWidth = Engine::get()->getWindow()->getWidth();
	int screenHeight = Engine::get()->getWindow()->getHeight();

		


	// Render menu bar
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File")) 
			{ // Start of File dropdown
				if (ImGui::MenuItem("Open Project", "Ctrl+O")) {


					const char* lTheSelectFolderName = tinyfd_selectFolderDialog(
						"let us just select a directory", "../../");

					//nfdchar_t* outPath = NULL;
					//nfdresult_t result = NFD_PickFolder(NULL, &outPath);
					if (lTheSelectFolderName)
					{
						Engine::get()->loadProject(lTheSelectFolderName);
						state.selectEntity(Entity::EmptyEntity);
						updateScene();
					}
				}
				if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
					Engine::get()->saveProject();
				}
				if (ImGui::BeginMenu("Import")) {
					if (ImGui::MenuItem("New Asset")) {
						const char* filepath = tinyfd_openFileDialog(
							"Select an asset to load",
							"",
							NULL,
							NULL,
							"",
							1);

						if (filepath)
						{
							std::vector<std::string> files;
							std::stringstream ss(filepath);
							std::string token;
							while (std::getline(ss, token, '|'))
								files.push_back(token);

							static const std::unordered_set<std::string> textureExts = { ".png", ".jpg", ".bmp", ".hdr", ".exr" };

							for (const auto& file : files)
							{
								std::filesystem::path path(file);
								std::string ext = path.extension().string();
								std::string stem = path.filename().stem().string();
								std::string uniqueName = Engine::get()->getSubSystem<UniqueNameManager>()->suggestUniqueName(stem, EditorState::Instance().getWorkingDir().path());

								AssetBuildDescriptor desc;
								desc.name = uniqueName;
								desc.targetDirectory = EditorState::Instance().getWorkingDir().path();

								if (ext == ".lua")
								{
									desc.aType = AssetType::LUA_SCRIPT;
									LuaScriptLoadDescriptor loadDesc;
									loadDesc.sourcePath = file;
									Engine::get()->getSubSystem<Assets>()->importAsset(desc, loadDesc);
								}
								else if (textureExts.count(ext))
								{
									desc.aType = AssetType::TEXTURE;
									TextureLoadDescriptor texDesc;
									texDesc.sourcePath = file;
									texDesc.usage = TextureSemantic::Color;
									Engine::get()->getSubSystem<Assets>()->importAsset(desc, texDesc);
								}
								else
								{
									logWarning("Import not implemented for extension: " + ext);
								}
							}
						}
					}
					if (ImGui::MenuItem("Model")) {
						DialogManager::Instance().modelImportDialog.activate();
					}
					if (ImGui::MenuItem("Texture")) {
						DialogManager::Instance().textureImportDialog.activate();
					}
					if (ImGui::MenuItem("Animation")) {
						DialogManager::Instance().animationImportDialog.activate();
					}
					if (ImGui::MenuItem("Lua Script")) {
						DialogManager::Instance().luaScriptImportDialog.activate();
					}
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Build", "")) {
					// Path to the Python script
					std::string pythonScriptPath = "../../scripts/build_shipping.py";

			// Determine asset, output, and solution paths
			std::string assetsFolder = Engine::get()->getInitParams().projectDir;
			std::string outputFolder = std::string(SGE_SOLUTION_DIR) + "/../Release";
			std::string solutionDir = std::string(SGE_SOLUTION_DIR);

			// Command to execute the Python script with project, output, and solution paths
			std::string command = "python \"" + pythonScriptPath + "\" \"" + assetsFolder + "\" \"" + outputFolder + "\" \"" + solutionDir + "\"";

			// Run the command
			std::system(command.c_str());

			//std::filesystem::create_directories("../Game/data");
					}
					if (ImGui::MenuItem("Reload config", "")) {
						Engine::get()->reloadEngineConfig();
					}
					ImGui::Separator(); // Optional: Add a separator
					if (ImGui::MenuItem("Quit", "Alt+F4")) {
						Engine::get()->stop();
					}
					ImGui::EndMenu(); // End of File dropdown
				}
				if (ImGui::BeginMenu("Edit")) {
					
					CreateMenu::display();

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Project")) {
					if (ImGui::MenuItem("Settings")) {
						DialogManager::Instance().projectSettingsDialog.activate();
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Help")) {
					// Help menu items
					ImGui::EndMenu();
				}
				ImGui::EndMainMenuBar();
			}
			
			ImGui::EndMainMenuBar();
		}

		ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton);
		static bool dockspace_initialized = false;
		if (!dockspace_initialized)
		{
			dockspace_initialized = true;
			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

			ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);
			ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
			ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.3f, nullptr, &dockspace_id);
			ImGuiID dock_id_top = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.05f, nullptr, &dockspace_id);

			if (ImGuiDockNode* topNode = ImGui::DockBuilderGetNode(dock_id_top))
{
				topNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoDockingOverMe;
}

			ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_id_left);
			ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
			ImGui::DockBuilderDockWindow("AssetInspector", dock_id_right);
            ImGui::DockBuilderDockWindow("Asset View", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Simulation Controls", dock_id_top);
            ImGui::DockBuilderDockWindow("View", dockspace_id);
            ImGui::DockBuilderFinish(dockspace_id);
		}

		// Render UI
		RenderSimulationControlView();
		SceneViewWindow::display();
		SceneHierarchyWindow::display();
		InspectorWindow::display();
		AssetInspectorWindow::display();
		AnimationGraphWindow::display();
        AnimationViewerWindow::display();
        AssetViewWindow::display();
        RenderConsoleWindow();
		DisplayDebugInfoWindow();
		EditorState::Instance().displayDialogs();

		
	}
};

void setStyleAndColors()
{
	// Todo move to .ini file
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = ImGui::GetStyle().Colors;

	// Backgrounds
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.12f, 0.14f, 1.00f); // #1b1f23
	colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.11f, 0.13f, 0.98f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);

	// Text
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.97f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.55f, 0.60f, 1.00f);

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.13f, 0.15f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);

	// Frames, Inputs
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.30f, 0.40f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.27f, 0.33f, 0.45f, 1.00f);

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.27f, 0.53f, 0.90f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.48f, 0.80f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.15f, 0.18f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.196f, 0.196f, 0.196f, 1.0f);

	// Buttons (more vivid, professional tone)
	colors[ImGuiCol_Button] = ImVec4(0.18f, 0.22f, 0.30f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.32f, 0.55f, 0.92f, 1.00f); // #4091f7
	colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.48f, 0.84f, 1.00f);

	// Interactive elements
	colors[ImGuiCol_SliderGrab] = ImVec4(0.27f, 0.56f, 0.92f, 1.00f); // softer accent
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.34f, 0.63f, 0.96f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.34f, 0.75f, 1.00f, 1.00f);

	// Headers (collapsing section, tree, etc.)
	colors[ImGuiCol_Header] = ImVec4(0.21f, 0.24f, 0.29f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.27f, 0.53f, 0.90f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.24f, 0.48f, 0.84f, 1.00f);

	// Misc
	colors[ImGuiCol_Separator] = ImVec4(0.29f, 0.31f, 0.34f, 0.60f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.55f, 0.90f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.25f, 0.55f, 0.90f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.25f, 0.55f, 0.90f, 0.95f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.60f, 0.95f, 0.35f);

	style.WindowPadding = ImVec2(10, 10);
	style.FramePadding = ImVec2(6, 4);
	style.ItemSpacing = ImVec2(10, 6);
	style.IndentSpacing = 20.0f;
	style.WindowRounding = 5.0f;
	style.FrameRounding = 4.0f;
	style.GrabRounding = 4.0f;
	style.ScrollbarRounding = 6.0f;
	style.WindowBorderSize = 1.0f;

	//ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->AddFontDefault();                    // normal font
	//io.Fonts->AddFontDefault()->FontSize = 14.0f; // or specify config

	//ImFont* smallFont = io.Fonts->AddFontDefault();
	//// optionally configure
	// ImFontConfig cfg;
	// cfg.SizePixels = 12.0f;
	// ImFont* smallFont = io.Fonts->AddFontDefault(&cfg);
}

static bool debugTerrainFlag = false;

class EditorApp : public Application
{
public:
	void start() override
	{
		EditorState::Instance().init();
		m_editorRegistry = std::make_shared<SGE_Regsitry>();
		
		auto gui = std::make_shared<ImguiHandler>();
		gui->init();
		gui->addGUI(new GUI_Helper);
		Engine::get()->addGUILayer(gui);

        setStyleAndColors();

        NativeScriptsLoader::instance->init();

		/*
		* 
		* Layer ordering:
		* 
		1. UI / ImGui layer (blocks everything if interacting with UI)
		2. ToolsLayer (active tool : TRS, foliage paint, terrain sculpt, etc.)
		3. SelectionLayer (entity picking, marquee, deselect)
		4. Camera / navigation layer
		5. Gameplay / scene input 
		*/
        Engine::get()->getEventSystem()->pushLayer(uiNavigationLayer);
        Engine::get()->getEventSystem()->pushLayer(uiEntitySelectionLayer);
        Engine::get()->getEventSystem()->pushLayer(uiEditorToolsLayer);
        Engine::get()->getEventSystem()->pushLayer(uiLayer);

		uiHandler = Engine::get()->getEventSystem()->bindToLayer(uiLayer->name);
		gameHandler = Engine::get()->getEventSystem()->bindToLayer("GameLayer");

		// set Editor camera as active camera
		auto editorCamera = m_editorRegistry->createEntity();
		editorCamera.addComponent<CameraComponent>(CameraComponent::createPerspectiveCamera(45.0f, (float)Engine::get()->getWindow()->getWidth() / Engine::get()->getWindow()->getHeight(), 0.1f, 3000.0f));
		editorCamera.addComponent<NativeScriptComponent>().bind<EditorCamera>();
		auto& nsc = editorCamera.getComponent<NativeScriptComponent>();
		nsc.script->onCreate();
		g_editorCamera = editorCamera;

		Engine::get()->getInput()->getKeyboard()->onKeyPressed(gameHandler, KeyCode::SCANCODE_ESCAPE, [](KeyPressedEvent e) { stopSimulation(); return false; });
		
		Engine::get()->getInput()->getKeyboard()->onKeyReleased(uiHandler, KeyCode::SCANCODE_X, [](KeyReleasedEvent e) {
			debugTerrainFlag = true;
			return false;
			});

		setupScene();
		updateScene();

		icons["model"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-cube-100.png");
		icons["texture"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-image-100.png");
		icons["animation"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-skeleton-100.png");
		icons["shader"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-pixel-100.png");
		icons["folder"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-folder-100.png");
		icons["prefab"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-blueprint-100.png");
		icons["material"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-material-100.png");
		icons["lua_script"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/icons8-code-file-100.png");
	}

	void update(float deltaTime) override
	{
		g_editorCamera.getComponent<NativeScriptComponent>().script->onUpdate(deltaTime);
		g_editorCamera.getComponent<Transformation>().update();

		EditorState::Instance().update();
	}
	std::shared_ptr<SGE_Regsitry> m_editorRegistry;
	
	EventHandler uiHandler;
	EventHandler gameHandler;
};

Application* CreateApplication()
{
	return new EditorApp();
}