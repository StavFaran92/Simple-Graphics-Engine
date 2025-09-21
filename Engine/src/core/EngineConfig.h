#pragma once 

#include <string>

struct EngineConfig
{
	struct RenderConfig
	{
		bool renderShadowMap = true;
		bool renderForwardPass = true;
		bool renderDeferredPass = true;
		bool renderCustomShadersPass = true;
		bool renderSkyboxPass = true;
		bool renderTerrainPass = true;
		bool renderVolumetricsPass = true;
		bool renderWireframeGrid = true;
		bool renderFoliagePass = true;
		bool renderNonOpaquePass = true;
	};

	EngineConfig(const std::string& path);

	void loadConfig();

	RenderConfig renderConfig;

	std::string configPath;
};