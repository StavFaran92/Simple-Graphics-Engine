#include "render/ShaderBuilder.h"

#include <sstream>

#include "core/Engine.h"
#include "render/ShaderLoader.h"

#include "utils/Utils.h"
#include "CommonTextures.h"
#include "core/Factory.h"

#include "Assets.h"

//enum class DirectiveType
//{
//    VERTEX,
//    FRAGMENT,
//    GEOMETRY,
//    COMPUTE,
//    UNIFORMS,
//
//    NONE
//};
//
//struct ShadersInfo
//{
//    std::string vertexCode;
//    std::string fragmentCode;
//    std::string geometryCode;
//    std::string computeCode;
//    std::string uniformsCode;
//};
//
//bool parseGLSLShader(const std::string& sCode, ShadersInfo& shader) {
//    std::stringstream stream(sCode);
//
//    std::string line, directive;
//    DirectiveType sType = DirectiveType::NONE;
//    std::map<DirectiveType, std::stringstream> directiveStreams;
//
//    while (std::getline(stream, line)) {
//        std::stringstream lineStream(line);
//        lineStream >> directive;
//
//        if (directive == "#vert") {
//            sType = DirectiveType::VERTEX;
//            continue;
//        }
//        else if (directive == "#frag") {
//            sType = DirectiveType::FRAGMENT;
//            continue;
//        }
//        else if (directive == "#geom") {
//            sType = DirectiveType::GEOMETRY;
//            continue;
//        }
//        else if (directive == "#compute") {
//            sType = DirectiveType::COMPUTE;
//            continue;
//        }
//
//        directiveStreams[sType] << line << '\n';
//    }
//
//    shader.vertexCode = directiveStreams[DirectiveType::VERTEX].str();
//    shader.fragmentCode = directiveStreams[DirectiveType::FRAGMENT].str();
//    shader.geometryCode = directiveStreams[DirectiveType::GEOMETRY].str();
//    shader.computeCode = directiveStreams[DirectiveType::COMPUTE].str();
//    shader.uniformsCode = directiveStreams[DirectiveType::UNIFORMS].str();
//
//    return true;
//}

//void addMacro(std::string& source, const std::string& macro) {
//	std::string macroDefinition = "#define " + macro + "\n";
//	size_t versionPos = source.find("#version");
//
//	if (versionPos != std::string::npos) {
//		// Find the end of the #version line
//		size_t lineEnd = source.find("\n", versionPos);
//		if (lineEnd != std::string::npos) {
//			// Insert the macro definition after the #version line
//			source.insert(lineEnd + 1, macroDefinition);
//		}
//		else {
//			// If no newline after #version, append the macro definition at the end
//			source.append(macroDefinition);
//		}
//	}
//	else {
//		// If no #version directive, prepend the macro definition
//		source = macroDefinition + source;
//	}
//}
//
//void replaceDirective(std::string& source, const std::string& directive, const std::string& replacement)
//{
//	size_t pos = source.find(directive);
//	if (pos != std::string::npos)
//	{
//		source.replace(pos, directive.length(), replacement);
//	}
//}

CustomShaderBuilder& CustomShaderBuilder::create(const std::string& filePath, ShaderOverride shaderOverride)
{
	CustomShaderBuilder* shaderBuilder = new CustomShaderBuilder(filePath, shaderOverride);



	return *shaderBuilder;
}

//#include <regex>
//
//void CustomShaderBuilder::parseUniforms(const std::string& shaderSource, Resource<Shader> shader)
//{
//	std::regex uniformRegex(R"(uniform\s+(\w+)\s+(\w+)\s*;)");
//	std::smatch match;
//	std::string::const_iterator searchStart(shaderSource.cbegin());
//
//	auto& uniformProperties = shader.m_uniformProperties;
//
//	while (std::regex_search(searchStart, shaderSource.cend(), match, uniformRegex)) {
//		std::string type = match[1].str();
//		std::string name = match[2].str();
//
//		if (type == "float") {
//			uniformProperties[name] = 0.0f;
//		}
//		else if (type == "vec2") {
//			uniformProperties[name] = glm::vec2(0.0f);
//		}
//		else if (type == "vec3") {
//			uniformProperties[name] = glm::vec3(0.0f);
//		}
//		else if (type == "vec4") {
//			uniformProperties[name] = glm::vec4(0.0f);
//		}
//		else if (type == "int") {
//			uniformProperties[name] = 0;
//		}
//		else if (type == "uint") {
//			uniformProperties[name] = 0u;
//		}
//		else if (type == "mat3") {
//			uniformProperties[name] = glm::mat3(1.0f);
//		}
//		else if (type == "mat4") {
//			uniformProperties[name] = glm::mat4(1.0f);
//		}
//		else if (type == "sampler2D") {
//			shader.addTexture(name, Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::WHITE_1X1));
//		}
//
//		searchStart = match.suffix().first;
//	}
//}

CustomShaderBuilder::CustomShaderBuilder(const std::string& filePath, ShaderOverride shaderOverride) : m_filepath(filePath) , m_shaderOverride(shaderOverride){ }

ShaderComponent CustomShaderBuilder::build()
{
	return {};
	//ShaderComponent shaderComponent;
	//

	//std::string& customShaderSource = Engine::get()->getShaderLoader()->readShader(m_filepath);

	//ShadersInfo customShaders;
	//Engine::get()->getShaderLoader()->parseGLSLShader(customShaderSource, customShaders);

	//parseUniforms(customShaderSource, shaderComponent);
	//
	//std::string shaderPath;
	//if (m_shaderOverride == ShaderOverride::PBR)
	//{
	//	shaderPath = SGE_ROOT_DIR + "Resources/Engine/Shaders/PBRShader.glsl";
	//}
	//else if (m_shaderOverride == ShaderOverride::Pixel)
	//{
	//	shaderPath = SGE_ROOT_DIR + "Resources/Engine/Shaders/PixelShader.glsl";
	//}

	//if (!customShaders.vertexCode.empty() || !customShaders.fragmentCode.empty())
	//{
	//	std::string& pixelShaderSources = Engine::get()->getShaderLoader()->readShader(shaderPath);

	//	ShadersInfo shaders;
	//	Engine::get()->getShaderLoader()->parseGLSLShader(pixelShaderSources, shaders);

	//	std::string macro = "CUSTOM_SHADER";

	//	if (!shaders.vertexCode.empty())
	//	{
	//		addMacro(shaders.vertexCode, macro);
	//		replaceDirective(shaders.vertexCode, "#custom_vert", customShaders.vertexCode);
	//	}

	//	if (!shaders.fragmentCode.empty())
	//	{
	//		addMacro(shaders.fragmentCode, macro);
	//		replaceDirective(shaders.fragmentCode, "#custom_frag", customShaders.fragmentCode);
	//	}

	//	//Shader* shader = new Shader();
	//	Resource<Shader> shader = Factory<Shader>::create();

	//	AssetInfo aInfo;
	//	aInfo.uuid = shader.getUID();
	//	aInfo.origFilePath = m_filepath;
	//	aInfo.aType = AssetType::SHADER;
	//	Engine::get()->getSubSystem<Assets>()->importAsset(aInfo);
	//	//Engine::get()->getSubSystem<Assets>()->addAsset(shader.getUID(), AssetType::SHADER); // this is a hack, we should load the copied shader.
	//	shader->BuildShaders(shaders);

	//	shaderComponent.m_customShader = shader;
	//	shaderComponent.shaderOverride = m_shaderOverride;
	//	shaderComponent.m_shaderFilePath = m_filepath;
	//	shaderComponent.isValid = true;

	//	// Save asset

	//}



	//delete this;

	//return shaderComponent;
}
