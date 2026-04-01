#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "memory/AssetHandle.h"
#include "render/Shader.h"
#include "render/RenderView.h"
#include "texture/Texture.h"

struct EngineAPI ShaderComponent : public Component
{
	enum ProjectionType : int
	{
		DefaultProjection = 0,
		Texture2D = 1
	};

	ShaderComponent();

	void addTexture(const std::string& name, AssetHandle<TextureAsset> texture)
	{
		customTextures[name] = texture;
	}

	void setProjectionTexture(AssetHandle<TextureAsset> texture);

	void update();

	void parseUniforms(const std::string& sourceCode);

	void parseFromShader(AssetHandle<ShaderAsset> shader);


	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_customShader);
		SERIALIZED_MEMBER(shaderOverride);
		SERIALIZED_MEMBER(customTextures);
		SERIALIZED_MEMBER(projection);
		SERIALIZED_MEMBER(projectionTexture);
		SERIALIZED_MEMBER(m_shaderFilePath);
		SERIALIZED_MEMBER(isValid);
	}

	// This will only be used by forward renderer, ignored by deffered
	AssetHandle<ShaderAsset> m_customShader;

	std::map<std::string, AssetHandle<TextureAsset>> customTextures;

	ProjectionType projection = ProjectionType::DefaultProjection;
	AssetHandle<TextureAsset> projectionTexture;
	std::shared_ptr<RenderView> renderViewProjection;

	std::unordered_map<std::string, Value> m_uniformProperties;

	std::string m_shaderFilePath;
	ShaderOverride shaderOverride;
	bool isValid = false;

};

REGISTER_COMPONENT(ShaderComponent)