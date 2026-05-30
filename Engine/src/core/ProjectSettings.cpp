#include "core/ProjectSettings.h"

#include <fstream>
#include <iomanip>

#include "core/Engine.h"
#include "core/Logger.h"

using json = nlohmann::json;

void to_json(json& j, const ProjectSettings& s)
{
	j = json{
		{"layerNames", s.m_layerNames}
	};
}

void from_json(const json& j, ProjectSettings& s)
{
	j.at("layerNames").get_to(s.m_layerNames);
}

std::string ProjectSettings::getLayerName(int index) const
{
	if (index >= static_cast<int>(m_layerNames.size()))
	{
		logWarning("Invalid layer: {}", index);
		return "";
	}
	return m_layerNames.at(index);
}

void ProjectSettings::setLayerName(int index, const std::string& name)
{
	if (index >= static_cast<int>(m_layerNames.size()))
	{
		logWarning("Invalid layer: {}", index);
		return;
	}
	m_layerNames[index] = name;
}

void ProjectSettings::create(const std::string& projectDir)
{
	init();

	std::string filePath = projectDir + m_fileName;

	json j;
	to_json(j, *this);

	std::ofstream file(filePath);
	if (!file.is_open())
	{
		logError("Failed to create project settings file: " + filePath);
		return;
	}

	file << std::setw(4) << j << std::endl;
	file.close();

	logInfo("Project settings file created: " + filePath);
}

void ProjectSettings::load(const std::string& projectDir)
{
	if (!parse(projectDir))
	{
		logInfo("Failed to locate a valid project settings in {}, creating a new one..", projectDir);
		create(projectDir);
	}
}

void ProjectSettings::save()
{
	std::string filePath = Engine::get()->getProjectDirectory() + m_fileName;

	json j;
	to_json(j, *this);

	std::ofstream file(filePath, std::ios::trunc);
	if (!file.is_open())
	{
		logError("Failed to save project settings file: " + filePath);
		return;
	}

	file << std::setw(4) << j << std::endl;
	file.close();
}

bool ProjectSettings::parse(const std::string& projectDir)
{
	std::string filePath = projectDir + m_fileName;

	std::ifstream file(filePath);
	if (!file.is_open())
	{
		logError("Failed to open project settings file: " + filePath);
		return false;
	}

	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	json j = json::parse(content, nullptr, false);
	if (j.is_discarded())
	{
		logError("Failed to parse project settings file: " + filePath);
		return false;
	}

	from_json(j, *this);

	return true;
}

void ProjectSettings::init()
{
	for (int i = 0; i < m_layerNames.size(); i++) {
		m_layerNames[i] = "Layer_" + std::to_string(i + 1);
	}
}
