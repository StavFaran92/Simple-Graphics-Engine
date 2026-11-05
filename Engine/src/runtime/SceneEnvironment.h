#pragma once

#include "memory/UUID.h"
#include "memory/ResourceRef.h"
#include "memory/AssetAliases.h"
#include "serialize/CerealHelpers.h"

// The scene's sky and environment lighting
struct SceneEnvironment
{
	// Cubemap texture asset; empty means the built-in procedural sky
	UUID skyCubemap;

	// Built by the scene from skyCubemap
	TextureResourceRef sky; // the background
	TextureResourceRef irradianceMap;
	TextureResourceRef prefilterEnvMap;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(skyCubemap);
	}
};
