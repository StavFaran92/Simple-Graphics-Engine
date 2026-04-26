#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "texture/Texture.h"

class Scene;

struct EngineAPI SkyboxComponent : public Component
{
	SkyboxComponent() = default;

	SkyboxComponent(TextureAssetRef skyboxImage);

	std::string getName() override { return "SkyboxComponent"; }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(originalImage);
		SERIALIZED_MEMBER(m_irradianceMap);
		SERIALIZED_MEMBER(m_prefilterEnvMap);
		SERIALIZED_MEMBER(m_cubemap);
		SERIALIZED_MEMBER(m_isBuilt);

	}

	void setSkybox(TextureAssetRef image);

	void build();

	void onInit(SceneResourceRef& scene) override;
	void resolve(SceneResourceRef& scene) override;
	void postLoad(SceneResourceRef& scene) override;


	TextureResourceRef cubemapIBL;
	//TextureResourceRef cubemap;

	//TextureAssetRef

	SceneResourceRef m_scene = nullptr;

	bool m_isBuilt = false;

	TextureAssetRef m_prefilterEnvMap;
	TextureAssetRef originalImage;
	TextureAssetRef m_cubemap;
	TextureAssetRef m_irradianceMap;

protected:
	std::vector<AssetRef<Asset>> gatherDependenciesInternal() const override;
};

REGISTER_COMPONENT(SkyboxComponent)