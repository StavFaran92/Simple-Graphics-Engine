#pragma once

#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct EngineAPI ShaderComponent : public Component
{
	enum ProjectionType : int
	{
		DefaultProjection = 0,
		Texture2D = 1
	};

	ShaderComponent();

	ShaderComponent(Shader* vertexShader, Shader* fragmentShader);

	void addTexture(const std::string& name, ResourceWrapper<Texture> texture)
	{
		customTextures[name] = texture;
	}

	void setProjectionTexture(ResourceWrapper<Texture> texture);

	void update();

	void parseUniforms(const std::string& sourceCode);

	void setShader(ResourceWrapper<Shader> shader);


	static void attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
	{
		(void)scene;
		if (auto sc = std::dynamic_pointer_cast<ShaderComponent>(c))
		{
			auto& shader = entityHandler.addComponent<ShaderComponent>(*sc);
			shader.update();
		}
	}

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
	ResourceWrapper<Shader> m_customShader;

	std::map<std::string, ResourceWrapper<Texture>> customTextures;

	ProjectionType projection = ProjectionType::DefaultProjection;
	ResourceWrapper<Texture> projectionTexture;
	std::shared_ptr<RenderView> renderViewProjection;

	std::unordered_map<std::string, Value> m_uniformProperties;

	std::string m_shaderFilePath;
	ShaderOverride shaderOverride;
	bool isValid = false;

};

REGISTER_COMPONENT(ShaderComponent)