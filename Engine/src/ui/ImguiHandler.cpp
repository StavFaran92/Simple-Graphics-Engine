#include "ImguiHandler.h"

#include "Menu.h"

#include "Logger.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "GuiMenu.h"
#include "ImGuizmo.h"

bool ImguiHandler::init(SDL_Window* window, const SDL_GLContext& context)
{
	const char* glsl_version = "#version 140";

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	m_imguiContext = ImGui::CreateContext();

	// Set ImGui flags
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Set style
	ImGui::StyleColorsLight();

	// Init ImGui_SDL
	if (!ImGui_ImplSDL2_InitForOpenGL(window, context))
	{
		logError("Init Imgui_SDL failed.");
		return false;
	}

	// Init Imgui_OpenGL3
	if(!ImGui_ImplOpenGL3_Init(glsl_version))
	{
		logError("Init Imgui_OpenGL failed.");
		return false;
	}

	logInfo("Imgui has initialized successfully.");

    return true;
}

void ImguiHandler::proccessEvents(SDL_Event& e)
{
	ImGui_ImplSDL2_ProcessEvent(&e);
}

void ImguiHandler::render()
{
	if (m_guiList.size() == 0) return;

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();

	for (auto& gui : m_guiList)
	{
		gui->display();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

bool ImguiHandler::close()
{


	for (int i = m_guiList.size() - 1; i >= 0; i--)
	{
		delete m_guiList[i];
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext((ImGuiContext*)m_imguiContext);


    return true;
}

void ImguiHandler::addGUI(GuiMenu* menu)
{
	m_guiList.push_back(menu);
}

void ImguiHandler::removeGUI(GuiMenu* menu)
{
	m_guiList.erase(std::remove(m_guiList.begin(), m_guiList.end(), menu), m_guiList.end());
}

void* ImguiHandler::getCurrentContext() const
{
	return m_imguiContext;
}

