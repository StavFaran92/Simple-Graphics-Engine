#pragma once

#include "core/Core.h"
#include "nlohmann/json.hpp"

#include <array>

class EngineAPI ProjectSettings
{
public:
	static ProjectSettings& get()
	{
		static ProjectSettings instance;
		return instance;
	}

	//Fields
	std::array<std::string, 32> m_layerNames;  // 32 bits max

	//Fields access
	std::string getLayerName(int index) const;
	void setLayerName(int index, const std::string& name);

	// Serialization
	void create(const std::string& projectDir);
	void save();
	void load(const std::string& projectDir);

private:
	ProjectSettings() = default;
	ProjectSettings(const ProjectSettings&) = delete;
	ProjectSettings& operator=(const ProjectSettings&) = delete;

private:
	bool parse(const std::string& projectDir);
	void init();
private:


	const std::string m_fileName = "ProjectSettings.json";
};

void to_json(nlohmann::json& j, const ProjectSettings& s);
void from_json(const nlohmann::json& j, ProjectSettings& s);
