#pragma once

#include "memory/AssetAliases.h"
#include "memory/ResourceRef.h"
#include "systems/SubSystem.h"

struct SceneEnvironment;

// Built-in gradient sky, shared by every scene without a sky cubemap
class EngineAPI ProceduralSky : public SubSystem
{
public:
	ProceduralSky() = default;
	ProceduralSky(const ProceduralSky&) = delete;
	ProceduralSky& operator=(const ProceduralSky&) = delete;

	// Renders the sky and its lighting maps; needs GL and the built-in meshes
	bool init();

	// Fills the environment's runtime maps
	void fill(SceneEnvironment& environment) const;

private:
	TextureResourceRef m_sky;
	TextureResourceRef m_irradianceMap;
	TextureResourceRef m_prefilterEnvMap;
};
