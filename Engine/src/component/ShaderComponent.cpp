#include <regex>

#include "component/ShaderComponent.h"

#include "memory/BuiltInAssets.h"

ShaderComponent::ShaderComponent()
{
	renderViewProjection = std::make_shared<RenderView>(Viewport{ 0, 0, 1920, 1080 }, Entity::EmptyEntity);
};

void ShaderComponent::setProjectionTexture(AssetHandle<Texture> texture)
{
	renderViewProjection->bind();
	renderViewProjection->setTexture(texture.resource());
	projectionTexture = texture;

}

void ShaderComponent::parseUniforms(const std::string& sourceCode)
{
	m_uniformProperties.clear();
	customTextures.clear();

	std::regex uniformRegex(R"(uniform\s+(\w+)\s+(\w+)\s*;)");
	std::smatch match;
	std::string::const_iterator searchStart(sourceCode.cbegin());

	auto& uniformProperties = m_uniformProperties;

	while (std::regex_search(searchStart, sourceCode.cend(), match, uniformRegex)) {
		std::string type = match[1].str();
		std::string name = match[2].str();

		if (type == "float") {
			uniformProperties[name] = 0.0f;
		}
		else if (type == "vec2") {
			uniformProperties[name] = glm::vec2(0.0f);
		}
		else if (type == "vec3") {
			uniformProperties[name] = glm::vec3(0.0f);
		}
		else if (type == "vec4") {
			uniformProperties[name] = glm::vec4(0.0f);
		}
		else if (type == "int") {
			uniformProperties[name] = 0;
		}
		else if (type == "uint") {
			uniformProperties[name] = 0u;
		}
		else if (type == "mat3") {
			uniformProperties[name] = glm::mat3(1.0f);
		}
		else if (type == "mat4") {
			uniformProperties[name] = glm::mat4(1.0f);
		}
		else if (type == "sampler2D") {
			customTextures[name] = BuiltInAssets::getByName<Texture>(SGE_TEXTURE_WHITE);
		}

		searchStart = match.suffix().first;
	}
}

void ShaderComponent::parseFromShader(AssetHandle<Shader> shader)
{
	m_customShader = shader;

	shaderOverride = shader.resource()->getShaderOverride();

	const std::string& sourceCode = shader.resource()->getSourceCode();

	parseUniforms(sourceCode);

	isValid = true;
}

void ShaderComponent::update()
{
	auto oldUniforms = m_uniformProperties;
	auto oldTextures = customTextures;

	parseUniforms(m_customShader.resource()->getSourceCode());


	auto& newTextures = customTextures;
	for (const auto [name, texture] : oldTextures)
	{
		auto iter = newTextures.find(name);
		if (iter != newTextures.end())
		{
			iter->second = texture;
		}
	}

	auto& newUniforms = m_uniformProperties;
	for (const auto [name, value] : oldUniforms)
	{
		auto iter = newUniforms.find(name);
		if (iter != newUniforms.end())
		{
			iter->second = value;
		}
	}

	for (const auto& [name, value] : m_uniformProperties)
	{
		m_customShader.resource()->setUniformValue(name, value);
	}

	if (!projectionTexture.resource().isEmpty())
	{
		setProjectionTexture(projectionTexture);
	}
}