#pragma once

#include "core/Engine.h"

#include <cstdint>
#include <string>
#include <vector>

struct SDL_Window;

// Welcome page shown before the engine starts when no project is given on the command line.
// Lists known projects (build/bin/<Config>/projects.json) and opens, creates, removes or deletes them.
class ProjectLauncher
{
public:
	// Shows the launcher until a project is chosen and fills params for Engine::init; false if it was closed
	static bool run(InitParams& params);

	// Adds the project to the list, or marks it as just opened
	static void rememberProject(const std::string& dir);

private:
	struct ProjectEntry
	{
		std::string name;
		std::string path;
		int64_t lastOpened = 0;
	};

	bool openWindow();
	void closeWindow();
	void drawUI();
	void drawProjectTable();
	void drawProjectButtons();
	void drawNewProject();
	void drawDeleteConfirm();

	void choose(const std::string& dir, bool existing);
	void addExisting();
	void createProject();
	void removeSelected(bool deleteFiles);

	static std::string listFilePath();
	static std::vector<ProjectEntry> loadList();
	static void saveList(const std::vector<ProjectEntry>& projects);
	static std::vector<ProjectEntry> projectsInSamples();
	static bool isProjectDir(const std::string& dir);
	static std::string normalize(const std::string& dir);

	InitParams* m_params = nullptr;
	SDL_Window* m_window = nullptr;
	void* m_glContext = nullptr;

	std::vector<ProjectEntry> m_projects;
	int m_selected = -1;
	bool m_done = false;
	bool m_chosen = false;

	char m_newName[128] = "";
	std::string m_newLocation;
	std::string m_error;
};
