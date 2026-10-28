#include "launcher/ProjectLauncher.h"
// Started without -e / -ec, the editor opens the project launcher first
#define SGE_PROJECT_LAUNCHER ProjectLauncher::run
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
#include "SimulationControls.h"
#include "EditorState.h"
#include "DialogManager.h"

#include "core/Logger.h"
#include "utils/STBIHelper.h"
#include "ViewCapture.h"
#include "ai/McpServer.h"
#include "ai/AgentTools.h"
#include "commands/CommandStack.h"
#include "commands/EntityCommands.h"
#include <vector>
#include <mutex>
#include <fstream>

#include "Common.h"
#include "AssetViewWindow.h"
#include "AgentWindow.h"

#include "EntityState.h"
#include "InspectorWindow.h"
#include "AssetInspectorWindow.h"
#include "AnimationGraphWindow.h"
#include "AnimationViewerWindow.h"
#include "SceneHierarchyWindow.h"
#include "SceneViewWindow.h"
#include "menus/CreateMenu.h"
#include "Thumbnails/ThumbnailCache.h"
#include "Thumbnails/TextureThumbnailGenerator.h"
#include "Thumbnails/MaterialThumbnailGenerator.h"


#include "ImguiHandler.h"
#include "GUIMenu.h"
#include "render/GpuInfo.h"

extern bool g_testRay;

static const std::string SGE_EDITOR_APP_ROOT = "../../EditorApp/Resources";
static GpuInfo g_gpuInfo;
std::shared_ptr<EventLayer> uiShortcutsLayer = std::make_shared<EventLayer>("uiShortcutsLayer");
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

void stopSimulation()
{
	EditorState::Instance().startButtonPressed = false;
	EditorState::Instance().simPaused = false;
	Engine::get()->getContext()->stopSimulation();

	Engine::get()->getContext()->getActiveScene()->setGameRenderViewEnabled(false);
	Engine::get()->getContext()->getActiveScene()->setRenderViewEnabled("Editor View", true);
	uiLayer->setEnabled(true);
	EditorState::Instance().isMouseLocked = false;
	static_cast<EditorCamera*>(g_editorCamera.getComponent<NativeScriptComponent>().script.get())->unlock(); //TODO this should be in camera event
}

void startsimulation()
{
	EditorState::Instance().startButtonPressed = true;
	EditorState::Instance().simPaused = false;

	if (EditorState::Instance().isMouseLocked)
	{
		Engine::get()->getWindow()->lockMouse();
	}

	Engine::get()->getContext()->startSimulation();

	uiLayer->setEnabled(false);
	Engine::get()->getContext()->getActiveScene()->setGameRenderViewEnabled(true);
	Engine::get()->getContext()->getActiveScene()->setRenderViewEnabled("Editor View", false);
	state.selectEntity(Entity::EmptyEntity);
	static_cast<EditorCamera*>(g_editorCamera.getComponent<NativeScriptComponent>().script.get())->lock(); //TODO this should be in camera event
}

void pauseSimulation()
{
	EditorState::Instance().simPaused = true;
	Engine::get()->getContext()->pauseSimulation();

	EditorState::Instance().isMouseLocked = Engine::get()->getWindow()->isMouseLocked();
	if (EditorState::Instance().isMouseLocked)
	{
		Engine::get()->getWindow()->unlockMouse();
	}
}

static void advanceSimulation()
{
	Engine::get()->getContext()->advanceSimulation();
}

static constexpr float kSimIconSize = 20.0f;

static bool simButton(const char* icon)
{
	return ImGui::ImageButton((ImTextureID)(intptr_t)icons.at(icon)->getID(), ImVec2(kSimIconSize, kSimIconSize));
}

void RenderSimulationControlView()
{
	ImGui::Begin("Simulation Controls", nullptr, windowFlags | ImGuiWindowFlags_NoResize);

	bool isRunning = EditorState::Instance().startButtonPressed;
	bool isPaused  = EditorState::Instance().simPaused;

	const bool isActive = Engine::get()->getContext()->getActiveScene()->isSimulationActive();

	float windowWidth = ImGui::GetContentRegionAvail().x;
	const float btnW = kSimIconSize + ImGui::GetStyle().FramePadding.x * 2.0f;
	const float btnH = kSimIconSize + ImGui::GetStyle().FramePadding.y * 2.0f;
	static const float gap  = 8.0f;
	ImGui::SetCursorPosX((windowWidth - btnW * 3.0f - gap * 2.0f) * 0.5f);
	// The dock strip is shorter than padding + buttons, so centre in the whole window to keep top and bottom equal
	ImGui::SetCursorPosY((ImGui::GetWindowHeight() - btnH) * 0.5f);

	// START — always calls context startSimulation
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (isActive ? 0.35f : 1.0f));
	if (simButton("sim_play"))
		startsimulation();
	ImGui::PopStyleVar();

	ImGui::SameLine(0, gap);

	// STOP — always calls context stopSimulation
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (isActive ? 1.0f : 0.35f));
	if (simButton("sim_stop"))
		stopSimulation();
	ImGui::PopStyleVar();

	ImGui::SameLine(0, gap);

	// PAUSE / ADVANCE toggle — greyed out when simulation is stopped
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (isRunning ? 1.0f : 0.35f));
	if (isPaused)
	{
		if (simButton("sim_advance") && isRunning)
			advanceSimulation();
	}
	else
	{
		if (simButton("sim_pause") && isRunning)
			pauseSimulation();
	}
	ImGui::PopStyleVar();


	ImGui::End();
}                                           

void LightCreatorWindow()
{
	if (EditorState::Instance().showLightCreatorWindow)
	{
		ImGui::SetNextWindowSize({ 400, 300 }, ImGuiCond_Appearing);
		ImGui::Begin("Light Creator");

		static LightType lightType = LightType::DirectionalLight;
		static float intensity = 1.0f;
		static glm::vec3 color(1.f, 1.f, 1.f);
		static glm::vec3 dir(0.f, 0.f, 0.f);
		static glm::vec3 pos(0.f, 0.f, 0.f);

		// Light Type
		ImGui::RadioButton("Directional Light", (int*)&lightType, 0);
		ImGui::RadioButton("Point Light", (int*)&lightType, 1);

		ImGui::ColorEdit3("Color", (float*)&color);
		ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 100.0f);

		if (lightType == LightType::DirectionalLight)
		{
			ImGui::InputFloat3("Direction", (float*)&dir);
		}
		if (lightType == LightType::PointLight)
		{
			ImGui::InputFloat3("Position", (float*)&pos);
		}

		if (ImGui::Button("Ok"))
		{
			if (lightType == LightType::DirectionalLight)
			{
				auto e = Engine::get()->getContext()->getActiveScene()->createEntity();
				e.addComponent<DirectionalLight>(color, intensity);
				CommandStack::Instance().record(std::make_unique<CreateEntitiesCmd>(e, "Create directional light"));
			}
			else if (lightType == LightType::PointLight)
			{
				auto e = Engine::get()->getContext()->getActiveScene()->createEntity();
				e.addComponent<PointLight>(color, intensity);
				CommandStack::Instance().record(std::make_unique<CreateEntitiesCmd>(e, "Create point light"));
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
        static bool s_consoleWasVisible = false;

        // Returns false while the console tab is hidden behind another docked tab
        const bool visible = ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus |
											ImGuiWindowFlags_NoCollapse |
											ImGuiWindowFlags_NoFocusOnAppearing |
											ImGuiWindowFlags_NoTitleBar |
											ImGuiWindowFlags_NoMove);

        // Jump to the most recent message whenever the console becomes visible
        const bool justBecameVisible = visible && !s_consoleWasVisible;
        s_consoleWasVisible = visible;

        if (visible)
        {
            // Stick to the bottom unless the user scrolled up; check before submitting new lines
            const bool atBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f;

            std::lock_guard<std::mutex> lock(g_consoleMutex);
            for (const auto& line : g_consoleLog)
            {
                ImGui::TextColored(LogLevelToColor(line.level), "%s", line.msg.c_str());
            }
            if (atBottom || justBecameVisible || g_scrollConsole)
            {
                ImGui::SetScrollHereY(1.0f);
                g_scrollConsole = false;
            }
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

	auto graphics = Engine::get()->getSubSystem<Graphics>();
	bool vsync = graphics->isVSyncEnabled();
	if (ImGui::Checkbox("VSync", &vsync))
		graphics->setVSync(vsync);

	ImGui::Separator();

	// --- GPU ---
	static float gpuInfoTimer = 0;
	gpuInfoTimer += Engine::get()->getSubSystem<System>()->getDeltaTime();
	if (gpuInfoTimer >= 0.5f)
	{
		g_gpuInfo.update();
		gpuInfoTimer = 0;
	}

	ImGui::Text("GPU: %s", g_gpuInfo.getRenderer().c_str());
	ImGui::Text("OpenGL %s, GLSL %s", g_gpuInfo.getGLVersion().c_str(), g_gpuInfo.getGLSLVersion().c_str());
	ImGui::Text("Max texture size: %d, texture units: %d", g_gpuInfo.getMaxTextureSize(), g_gpuInfo.getMaxTextureUnits());

	if (g_gpuInfo.hasMemoryInfo())
	{
		int used = g_gpuInfo.getUsedMemoryMB();
		int total = g_gpuInfo.getTotalMemoryMB();
		ImGui::Text("VRAM: %d / %d MB", used, total);
		ImGui::ProgressBar(total > 0 ? (float)used / total : 0.0f, ImVec2(-1, 0));
		ImGui::Text("Evictions: %d (%d MB)", g_gpuInfo.getEvictionCount(), g_gpuInfo.getEvictedMemoryMB());
	}

	ImGui::Separator();

	// --- CPU Scope Timers ---
	ImGui::Text("CPU Timings:");
	auto profilerSnapshot = Engine::get()->getSubSystem<ProfilerSystem>()->getSnapshot();
	for (const auto& [name, history] : profilerSnapshot)
	{
		if (history.count == 0)
			continue;

		ImGui::Text("%s: %.3f ms", name.c_str(), history.latest);

		const int valuesOffset = (history.count < ProfilerSystem::kHistorySize) ? 0 : history.offset;
		ImGui::PlotLines(("##" + name).c_str(),
			history.samples.data(),
			history.count,
			valuesOffset,
			nullptr,
			0.0f,
			FLT_MAX,
			ImVec2(0, 60));
	}

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
						ProjectLauncher::rememberProject(lTheSelectFolderName);
						CommandStack::Instance().clear();
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
					auto& commands = CommandStack::Instance();
					const std::string undoLabel = commands.canUndo() ? "Undo " + commands.undoLabel() : "Undo";
					const std::string redoLabel = commands.canRedo() ? "Redo " + commands.redoLabel() : "Redo";

					if (ImGui::MenuItem(undoLabel.c_str(), "Ctrl+Z", false, commands.canUndo()))
						commands.undo();
					if (ImGui::MenuItem(redoLabel.c_str(), "Ctrl+Y", false, commands.canRedo()))
						commands.redo();
					ImGui::Separator();

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
			ImGuiID dock_id_left_bottom = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.5f, nullptr, &dock_id_left);
			ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.2f, nullptr, &dockspace_id);
			ImGuiID dock_id_right_bottom = ImGui::DockBuilderSplitNode(dock_id_right, ImGuiDir_Down, 0.25f, nullptr, &dock_id_right);
			ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.3f, nullptr, &dockspace_id);
			ImGuiID dock_id_top = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.05f, nullptr, &dockspace_id);

			if (ImGuiDockNode* topNode = ImGui::DockBuilderGetNode(dock_id_top))
{
				topNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoDockingOverMe;
}

			ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_id_left);
			ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
			ImGui::DockBuilderDockWindow("Agent", dock_id_right);
			ImGui::DockBuilderDockWindow("Debug Info", dock_id_right_bottom);
			ImGui::DockBuilderDockWindow("AssetInspector", dock_id_left_bottom);
            ImGui::DockBuilderDockWindow("Asset View", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
            ImGui::DockBuilderDockWindow("Simulation Controls", dock_id_top);
            ImGui::DockBuilderDockWindow("View", dockspace_id);
            ImGui::DockBuilderFinish(dockspace_id);
		}

		// Before any window draws: evicts stale thumbnails and generates a few pending ones
		ThumbnailCache::Instance().update();

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
		AgentWindow::display();
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

	// Tables
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.16f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.29f, 0.31f, 0.34f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);

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

	// The first font added becomes the default; without the file ImGui keeps its built-in one
	const char* uiFont = SGE_ROOT_DIR "Resources/Engine/Fonts/Inter-Regular.ttf";
	if (fs::exists(uiFont))
		ImGui::GetIO().Fonts->AddFontFromFileTTF(uiFont, 15.0f);
}

static bool debugTerrainFlag = false;

// Capture mode (-capture / -log / -frames): lets tools and AI agents run the editor and read back what it showed
static int g_captureFrame = 0;

static void createParentDirs(const std::string& path)
{
	fs::path parent = fs::path(path).parent_path();
	if (!parent.empty())
		fs::create_directories(parent);
}

static void writeEditorViewPng(const std::string& path)
{
	CapturedImage image;
	if (!captureRenderView("Editor View", image))
	{
		logWarning("Capture: could not read the 'Editor View' texture");
		return;
	}

	createParentDirs(path);
	STBIHelper::writeToPNG(path, image.width, image.height, 3, image.rgb.data(), image.width * 3);
	logInfo("Capture: wrote " + path);
}

static void writeConsoleLog(const std::string& path)
{
	createParentDirs(path);
	std::ofstream out(path);

	std::lock_guard<std::mutex> lock(g_consoleMutex);
	// Messages arrive formatted ("[time] [level] [file:line] text") with trailing newlines
	for (const auto& line : g_consoleLog)
	{
		const size_t end = line.msg.find_last_not_of("\r\n");
		if (end != std::string::npos)
			out << line.msg.substr(0, end + 1) << "\n";
	}
}

static void updateCaptureMode()
{
	const InitParams& params = Engine::get()->getInitParams();
	if (params.captureFrames <= 0 || ++g_captureFrame != params.captureFrames)
		return;

	if (!params.capturePath.empty())
		writeEditorViewPng(params.capturePath);
	if (!params.logPath.empty())
		writeConsoleLog(params.logPath);

	Engine::get()->stop();
}

class EditorApp : public Application
{
public:
	void start() override
	{
		EditorState::Instance().init();
		g_gpuInfo.init();
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
        Engine::get()->getEventSystem()->pushLayer(uiShortcutsLayer);

		uiShortucutsHandler = Engine::get()->getEventSystem()->bindToLayer(uiShortcutsLayer->name);
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

		auto* kb = Engine::get()->getInput()->getKeyboard();

		// F9 — Start
		kb->onKeyPressed(uiShortucutsHandler, KeyCode::SCANCODE_F9, [](KeyPressedEvent e) {
			startsimulation();
			return true;
		});

		// F10 — Stop
		kb->onKeyPressed(uiShortucutsHandler, KeyCode::SCANCODE_F10, [](KeyPressedEvent e) {
			stopSimulation();
			return true;
		});

		// F11 — Pause / Advance
		kb->onKeyPressed(uiShortucutsHandler, KeyCode::SCANCODE_F11, [](KeyPressedEvent e) {
			if (!EditorState::Instance().startButtonPressed) return false;
			if (EditorState::Instance().simPaused)
				advanceSimulation();
			else
				pauseSimulation();
			return true;
		});

		// Ctrl+Z / Ctrl+Y — Undo / Redo; a focused text field keeps its own Ctrl+Z
		auto ctrlShortcut = [kb] {
			const bool ctrl = kb->getKeyState(KeyCode::SCANCODE_LCTRL) || kb->getKeyState(KeyCode::SCANCODE_RCTRL);
			return ctrl && !ImGui::GetIO().WantTextInput;
		};
		kb->onKeyPressed(uiShortucutsHandler, KeyCode::SCANCODE_Z, [ctrlShortcut](KeyPressedEvent e) {
			if (!ctrlShortcut()) return false;
			CommandStack::Instance().undo();
			return true;
		});
		kb->onKeyPressed(uiShortucutsHandler, KeyCode::SCANCODE_Y, [ctrlShortcut](KeyPressedEvent e) {
			if (!ctrlShortcut()) return false;
			CommandStack::Instance().redo();
			return true;
		});

		Engine::get()->getInput()->getKeyboard()->onKeyReleased(uiShortucutsHandler, KeyCode::SCANCODE_X, [](KeyReleasedEvent e) {
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
		icons["sim_play"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/material-play-arrow-96.png");
		icons["sim_stop"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/material-stop-96.png");
		icons["sim_pause"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/material-pause-96.png");
		icons["sim_advance"] = Texture::load(SGE_EDITOR_APP_ROOT + "/Content/Textures/material-skip-next-96.png");

		// Projects opened with -e / -ec show up in the launcher too; a temp dir isn't a project to come back to
		if (!Engine::get()->getInitParams().tempDir)
			ProjectLauncher::rememberProject(Engine::get()->getProjectDirectory());

		ThumbnailCache::Instance().getRegistry().registerGenerator(AssetType::TEXTURE, std::make_unique<TextureThumbnailGenerator>());
		ThumbnailCache::Instance().getRegistry().registerGenerator(AssetType::MATERIAL, std::make_unique<MaterialThumbnailGenerator>());

		// Crash reports end with the console tail; it includes the [MCP] lines of the agent's last tool calls
		ErrorHandler::setCrashContext([] {
			constexpr size_t kTailLines = 50;
			std::unique_lock<std::mutex> lock(g_consoleMutex, std::try_to_lock);
			if (!lock.owns_lock())
				return std::string("Console: busy while crashing\n");

			std::string text = "Last console lines:\n";
			const size_t first = g_consoleLog.size() > kTailLines ? g_consoleLog.size() - kTailLines : 0;
			for (size_t i = first; i < g_consoleLog.size(); ++i)
			{
				const std::string& msg = g_consoleLog[i].msg;
				text += "  " + msg.substr(0, msg.find_last_not_of("\r\n") + 1) + "\n";
			}
			return text;
		});

		// Capture runs are short scripted sessions; leave the port to an editor the user has open
		if (Engine::get()->getInitParams().captureFrames <= 0)
		{
			registerAgentTools(m_mcpServer, [] {
				std::lock_guard<std::mutex> lock(g_consoleMutex);
				std::vector<ConsoleLine> lines;
				lines.reserve(g_consoleLog.size());
				for (const auto& line : g_consoleLog)
					lines.push_back({ static_cast<int>(line.level), line.msg });
				return lines;
			});
			m_mcpServer.start(kMcpPort);
		}
	}

	void update(float deltaTime) override
	{
		g_editorCamera.getComponent<NativeScriptComponent>().script->onUpdate(deltaTime);
		g_editorCamera.getComponent<Transformation>().update();

		EditorState::Instance().update();

		// AI tool calls run here, on the main thread, after this frame's scene was drawn
		m_mcpServer.queue().drain();

		// Runs after the scene is drawn this frame, so the Editor View holds the current image
		updateCaptureMode();
	}

	void close() override
	{
		m_mcpServer.stop();
	}

	static constexpr int kMcpPort = 7777;
	McpServer m_mcpServer;

	std::shared_ptr<SGE_Regsitry> m_editorRegistry;
	
	EventHandler uiHandler;
	EventHandler uiShortucutsHandler;
	EventHandler gameHandler;
};

Application* CreateApplication()
{
	return new EditorApp();
}