#include "EngineConfig.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

EngineConfig::EngineConfig(const std::string& path)
    : configPath(path)
{
	loadConfig();
}

void EngineConfig::loadConfig()
{
    std::ifstream inFile(configPath);
    if (!inFile)
    {
        throw std::runtime_error("Could not locate config path: " + configPath);
    }

    json j;
    inFile >> j;

    if (j.contains("Rendering")) {
        const auto& render = j["Rendering"];
        if (render.contains("RenderForwardPass"))
            renderConfig.renderForwardPass = render["RenderForwardPass"].get<bool>();
        if (render.contains("RenderShadowMap"))
            renderConfig.renderShadowMap = render["RenderShadowMap"].get<bool>();
        if (render.contains("RenderDeferredPass"))
            renderConfig.renderDeferredPass = render["RenderDeferredPass"].get<bool>();
        if (render.contains("RenderCustomShadersPass"))
            renderConfig.renderCustomShadersPass = render["RenderCustomShadersPass"].get<bool>();
        if (render.contains("RenderSkyboxPass"))
            renderConfig.renderSkyboxPass = render["RenderSkyboxPass"].get<bool>();
        if (render.contains("RenderTerrainPass"))
            renderConfig.renderTerrainPass = render["RenderTerrainPass"].get<bool>();
        if (render.contains("RenderVolumetricsPass"))
            renderConfig.renderVolumetricsPass = render["RenderVolumetricsPass"].get<bool>();
        if (render.contains("RenderWireframeGrid"))
            renderConfig.renderWireframeGrid = render["RenderWireframeGrid"].get<bool>();
    }
}
