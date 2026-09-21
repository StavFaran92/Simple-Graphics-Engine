#include "render/Graphics.h"

#include "core/Engine.h"
#include "core/Logger.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "memory/BuiltInAssets.h"
#include "memory/BuiltInResources.h"
#include "texture/Texture.h"

#include "SDL2/SDL.h"

Graphics::Graphics()
{
	constexpr int maxInstancedObjects = 5000; // approx upper bound on instanced object count
	constexpr int maxBonesPerObject = 100; // matches MAX_BONES in animation.glsl

	instancedModelBuffer.allocate(maxInstancedObjects * sizeof(glm::mat4));
	instancedAnimationBuffer.allocate(maxInstancedObjects * maxBonesPerObject * sizeof(glm::mat4));
	instancedInstanceDataBuffer.allocate(maxInstancedObjects * sizeof(InstanceData));
	//irradianceMap = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
	//prefilterEnvMap = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
	//brdfLUT = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
	//shadowMap = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
	//ssaoTexture = BuiltInAssets::getByName<TextureAsset>(SGE_TEXTURE_WHITE).resource();
}

void Graphics::setVSync(bool enabled)
{
	// 1 = wait for vertical retrace, 0 = present immediately.
	if (SDL_GL_SetSwapInterval(enabled ? 1 : 0) != 0)
	{
		logError("Unable to set VSync! SDL Error: {}", SDL_GetError());
		return;
	}

	m_vsync = enabled;
}

void Graphics::reloadShaders()
{
	Engine::get()->getSubSystem<BuiltInResources>()->loadAllResources();
}
