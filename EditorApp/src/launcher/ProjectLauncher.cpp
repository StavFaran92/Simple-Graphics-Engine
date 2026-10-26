#include "ProjectLauncher.h"

#include "Common.h"
#include "systems/UniqueNameManager.h"

#include <GL/glew.h>
#include "SDL2/SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include "tinyfiledialogs.h"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace
{
	// A folder is a project once the engine has written its asset registry there
	constexpr const char* kProjectMarker = "ProjectAssetRegistry.json";

	int64_t now()
	{
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	std::string formatTime(int64_t seconds)
	{
		if (seconds <= 0)
			return "-";

		const std::time_t time = static_cast<std::time_t>(seconds);
		std::tm local{};
		localtime_s(&local, &time);
		char buffer[32];
		std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &local);
		return buffer;
	}
}

bool ProjectLauncher::run(InitParams& params)
{
	ProjectLauncher launcher;
	launcher.m_params = &params;
	launcher.m_projects = loadList();
	launcher.m_newLocation = normalize(SGE_ROOT_DIR "../samples");

	if (!launcher.openWindow())
	{
		tinyfd_messageBox("SGE", (std::string("The project launcher couldn't open: ") + SDL_GetError()).c_str(), "ok", "error", 1);
		return false;
	}

	while (!launcher.m_done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE))
				launcher.m_done = true;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		launcher.drawUI();
		ImGui::Render();

		int width = 0, height = 0;
		SDL_GL_GetDrawableSize(launcher.m_window, &width, &height);
		glViewport(0, 0, width, height);
		const ImVec4 background = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		glClearColor(background.x, background.y, background.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(launcher.m_window);
	}

	launcher.closeWindow();
	return launcher.m_chosen;
}

void ProjectLauncher::rememberProject(const std::string& dir)
{
	const std::string path = normalize(dir);
	std::vector<ProjectEntry> projects = loadList();

	auto it = std::find_if(projects.begin(), projects.end(), [&](const ProjectEntry& p) { return p.path == path; });
	if (it == projects.end())
		projects.push_back({ fs::path(path).filename().string(), path, now() });
	else
		it->lastOpened = now();

	saveList(projects);
}

bool ProjectLauncher::openWindow()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		return false;

	// The same context version as the engine's window (Window::init)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	const auto flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	m_window = SDL_CreateWindow("SGE - Projects", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 560, flags);
	if (!m_window)
		return false;

	m_glContext = SDL_GL_CreateContext(m_window);
	if (!m_glContext)
	{
		SDL_DestroyWindow(m_window);
		return false;
	}

	SDL_GL_MakeCurrent(m_window, m_glContext);
	SDL_GL_SetSwapInterval(1);
	glewExperimental = GL_TRUE;
	glewInit();

	ImGui::CreateContext();
	// The editor keeps its window layout in imgui.ini; the launcher mustn't overwrite it
	ImGui::GetIO().IniFilename = nullptr;
	ImGui::StyleColorsLight();
	setStyleAndColors();
	ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
	ImGui_ImplOpenGL3_Init("#version 140");
	return true;
}

void ProjectLauncher::closeWindow()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	SDL_GL_DeleteContext(m_glContext);
	SDL_DestroyWindow(m_window);
}

void ProjectLauncher::drawUI()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::Begin("Projects", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

	ImGui::Text("Simple Graphics Engine");
	ImGui::TextDisabled("Open a project, or create a new one");
	ImGui::Separator();

	drawProjectTable();
	drawProjectButtons();
	ImGui::Separator();
	drawNewProject();

	if (!m_error.empty())
		ImGui::TextColored(ImVec4(0.8f, 0.1f, 0.1f, 1.0f), "%s", m_error.c_str());

	drawDeleteConfirm();
	ImGui::End();
}

void ProjectLauncher::drawProjectTable()
{
	// Leaves room below for the buttons, the new project row and an error line
	const ImVec2 size(0, -ImGui::GetFrameHeightWithSpacing() * 5);
	if (!ImGui::BeginTable("projects", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_ScrollY, size))
		return;

	ImGui::TableSetupScrollFreeze(0, 1);
	ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 200.0f);
	ImGui::TableSetupColumn("Path");
	ImGui::TableSetupColumn("Last opened", ImGuiTableColumnFlags_WidthFixed, 130.0f);
	ImGui::TableHeadersRow();

	for (int i = 0; i < (int)m_projects.size(); ++i)
	{
		const ProjectEntry& project = m_projects[i];
		const bool missing = !isProjectDir(project.path);

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		if (missing)
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.45f);

		ImGui::PushID(i);
		const auto flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick;
		if (ImGui::Selectable(project.name.c_str(), m_selected == i, flags))
		{
			m_selected = i;
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && !missing)
				choose(project.path, true);
		}
		ImGui::PopID();

		ImGui::TableNextColumn();
		ImGui::TextUnformatted(missing ? (project.path + "  (missing)").c_str() : project.path.c_str());
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(formatTime(project.lastOpened).c_str());

		if (missing)
			ImGui::PopStyleVar();
	}

	ImGui::EndTable();
}

void ProjectLauncher::drawProjectButtons()
{
	const bool hasSelection = m_selected >= 0 && m_selected < (int)m_projects.size();
	const bool selectedExists = hasSelection && isProjectDir(m_projects[m_selected].path);

	ImGui::BeginDisabled(!selectedExists);
	if (ImGui::Button("Open"))
		choose(m_projects[m_selected].path, true);
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(!hasSelection);
	if (ImGui::Button("Remove"))
		removeSelected(false);
	ImGui::EndDisabled();

	ImGui::SameLine();
	ImGui::BeginDisabled(!selectedExists);
	if (ImGui::Button("Delete..."))
		ImGui::OpenPopup("Delete project");
	ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Add Existing..."))
		addExisting();
}

void ProjectLauncher::drawNewProject()
{
	ImGui::Text("New project");

	ImGui::SetNextItemWidth(200.0f);
	ImGui::InputTextWithHint("##name", "Name", m_newName, sizeof(m_newName));
	ImGui::SameLine();
	ImGui::TextDisabled("in");
	ImGui::SameLine();
	ImGui::TextUnformatted(m_newLocation.c_str());

	ImGui::SameLine();
	if (ImGui::Button("Browse..."))
	{
		if (const char* folder = tinyfd_selectFolderDialog("Location of the new project", m_newLocation.c_str()))
			m_newLocation = normalize(folder);
	}

	ImGui::SameLine();
	ImGui::BeginDisabled(m_newName[0] == '\0');
	if (ImGui::Button("Create"))
		createProject();
	ImGui::EndDisabled();
}

void ProjectLauncher::drawDeleteConfirm()
{
	if (!ImGui::BeginPopupModal("Delete project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		return;

	if (m_selected >= 0 && m_selected < (int)m_projects.size())
	{
		ImGui::Text("Delete '%s' and all its files?", m_projects[m_selected].name.c_str());
		ImGui::TextDisabled("%s", m_projects[m_selected].path.c_str());
		ImGui::Text("This can't be undone.");
	}

	if (ImGui::Button("Delete"))
	{
		removeSelected(true);
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}

void ProjectLauncher::choose(const std::string& dir, bool existing)
{
	m_params->projectDir = dir;
	m_params->loadExistingProject = existing;
	m_chosen = true;
	m_done = true;
}

void ProjectLauncher::addExisting()
{
	const char* folder = tinyfd_selectFolderDialog("Open an existing project", m_newLocation.c_str());
	if (!folder)
		return;

	const std::string dir = normalize(folder);
	if (!isProjectDir(dir))
	{
		m_error = "'" + dir + "' isn't a project: it has no " + kProjectMarker;
		return;
	}

	rememberProject(dir);
	m_projects = loadList();
	m_selected = 0;
	m_error.clear();
}

void ProjectLauncher::createProject()
{
	const std::string name = m_newName;
	const std::string nameError = UniqueNameManager::fileNameError(name);
	if (!nameError.empty())
	{
		m_error = "The project name " + nameError;
		return;
	}

	// The engine sets up a new project only in an empty folder
	const fs::path dir = fs::path(m_newLocation) / name;
	std::error_code ec;
	if (fs::exists(dir, ec) && !fs::is_empty(dir, ec))
	{
		m_error = "'" + dir.generic_string() + "' already exists and isn't empty";
		return;
	}

	fs::create_directories(dir, ec);
	if (ec)
	{
		m_error = "Couldn't create '" + dir.generic_string() + "': " + ec.message();
		return;
	}

	choose(normalize(dir.string()), false);
}

void ProjectLauncher::removeSelected(bool deleteFiles)
{
	if (m_selected < 0 || m_selected >= (int)m_projects.size())
		return;

	const ProjectEntry project = m_projects[m_selected];
	if (deleteFiles)
	{
		// Only a folder the engine made into a project, never an arbitrary one
		if (!isProjectDir(project.path))
		{
			m_error = "'" + project.path + "' isn't a project; nothing was deleted";
			return;
		}

		std::error_code ec;
		fs::remove_all(project.path, ec);
		if (ec)
		{
			m_error = "Couldn't delete '" + project.path + "': " + ec.message();
			return;
		}
	}

	m_projects.erase(m_projects.begin() + m_selected);
	saveList(m_projects);
	m_selected = -1;
	m_error.clear();
}

std::string ProjectLauncher::listFilePath()
{
	// Next to the exe (build/bin/<Config>), so each build keeps its own list
	char* base = SDL_GetBasePath();
	const std::string dir = base ? base : "";
	SDL_free(base);
	return dir + "projects.json";
}

std::vector<ProjectLauncher::ProjectEntry> ProjectLauncher::loadList()
{
	std::ifstream file(listFilePath());
	if (!file)
	{
		// First run: start with the sample projects
		std::vector<ProjectEntry> seeded = projectsInSamples();
		saveList(seeded);
		return seeded;
	}

	std::vector<ProjectEntry> projects;
	try
	{
		const json list = json::parse(file);
		for (const json& p : list.value("projects", json::array()))
			projects.push_back({ p.value("name", ""), p.value("path", ""), p.value("lastOpened", int64_t(0)) });
	}
	catch (const json::exception&)
	{
		// A broken list starts empty rather than keeping the launcher from opening
	}

	std::stable_sort(projects.begin(), projects.end(), [](const ProjectEntry& a, const ProjectEntry& b) { return a.lastOpened > b.lastOpened; });
	return projects;
}

void ProjectLauncher::saveList(const std::vector<ProjectEntry>& projects)
{
	json list = { {"projects", json::array()} };
	for (const ProjectEntry& p : projects)
		list["projects"].push_back({ {"name", p.name}, {"path", p.path}, {"lastOpened", p.lastOpened} });

	std::ofstream file(listFilePath());
	file << list.dump(2);
}

std::vector<ProjectLauncher::ProjectEntry> ProjectLauncher::projectsInSamples()
{
	std::vector<ProjectEntry> projects;
	std::error_code ec;
	for (const auto& entry : fs::directory_iterator(SGE_ROOT_DIR "../samples", ec))
	{
		if (entry.is_directory() && isProjectDir(entry.path().string()))
			projects.push_back({ entry.path().filename().string(), normalize(entry.path().string()), 0 });
	}
	return projects;
}

bool ProjectLauncher::isProjectDir(const std::string& dir)
{
	std::error_code ec;
	return fs::exists(fs::path(dir) / kProjectMarker, ec);
}

std::string ProjectLauncher::normalize(const std::string& dir)
{
	std::error_code ec;
	fs::path path = fs::weakly_canonical(fs::path(dir), ec);
	if (ec)
		path = fs::path(dir);

	std::string result = path.generic_string();
	while (result.size() > 1 && result.back() == '/')
		result.pop_back();
	return result;
}
