#include "render/Graphics.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"
#include "memory/BuiltInAssets.h"
#include "memory/BuiltInResources.h"
#include "texture/Texture.h"

Graphics::Graphics()
{
	constexpr int maxInstancedObjects = 5000; // approx upper bound on instanced object count
	constexpr int maxBonesPerObject = 100; // matches MAX_BONES in animation.glsl

	instancedModelBuffer.allocate(maxInstancedObjects);
	instancedAnimationBuffer.allocate(maxInstancedObjects * maxBonesPerObject * sizeof(glm::mat4));
	instancedInstanceDataBuffer.allocate(maxInstancedObjects * sizeof(InstanceData));
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
