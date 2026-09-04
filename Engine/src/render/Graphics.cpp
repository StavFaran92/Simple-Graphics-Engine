#include "render/Graphics.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "memory/BuiltInAssets.h"
#include "memory/BuiltInResources.h"
#include "texture/Texture.h"

Graphics::Graphics()
{
	Engine::get()->registerSubSystem<Graphics>(this);

	//irradianceMap = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
	//prefilterEnvMap = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
	//brdfLUT = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
	//shadowMap = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
	//ssaoTexture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
}

void Graphics::reloadShaders()
{
	Engine::get()->getSubSystem<BuiltInResources>()->loadAllResources();
}
