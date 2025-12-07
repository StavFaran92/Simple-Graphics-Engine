#include "render/Graphics.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "render/DeferredRenderer.h"
#include "memory/BuiltInAssets.h"

Graphics::Graphics()
{
	Engine::get()->registerSubSystem<Graphics>(this);
}

void Graphics::reloadShaders()
{
	BuiltInAssets::getByName<Shader>(SGE_SHADER_TERRAIN).reimportAsset();
	BuiltInAssets::getByName<Shader>(SGE_SHADER_DEFFERED_PBR_GEOM).reimportAsset();
	BuiltInAssets::getByName<Shader>(SGE_SHADER_DEFFERED_PBR_LIGHT).reimportAsset();
	BuiltInAssets::getByName<Shader>(SGE_SHADER_FORWARD_PBR).reimportAsset();
	BuiltInAssets::getByName<Shader>(SGE_SHADER_DEBUG_DATA).reimportAsset();
}
