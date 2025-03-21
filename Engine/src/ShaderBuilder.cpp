#include "ShaderBuilder.h"

#include <sstream>

#include "Engine.h"
#include "ShaderLoader.h"

#include "Utils.h"

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

void addMacro(std::string& source, const std::string& macro) {
	std::string macroDefinition = "#define " + macro + "\n";
	size_t versionPos = source.find("#version");

	if (versionPos != std::string::npos) {
		// Find the end of the #version line
		size_t lineEnd = source.find("\n", versionPos);
		if (lineEnd != std::string::npos) {
			// Insert the macro definition after the #version line
			source.insert(lineEnd + 1, macroDefinition);
		}
		else {
			// If no newline after #version, append the macro definition at the end
			source.append(macroDefinition);
		}
	}
	else {
		// If no #version directive, prepend the macro definition
		source = macroDefinition + source;
	}
}

void replaceDirective(std::string& source, const std::string& directive, const std::string& replacement)
{
	size_t pos = source.find(directive);
	if (pos != std::string::npos)
	{
		source.replace(pos, directive.length(), replacement);
	}
}

std::string embedCustomShaderInUberShader(std::string& customShaderSource, std::string& uberShaderSource)
{
	

	return uberShaderSource;
}



CustomShaderBuilder& CustomShaderBuilder::create(const std::string& filePath, ShaderOverride shaderOverride)
{
	CustomShaderBuilder* shaderBuilder = new CustomShaderBuilder(filePath, shaderOverride);



	return *shaderBuilder;
}

CustomShaderBuilder::CustomShaderBuilder(const std::string& filePath, ShaderOverride shaderOverride) : m_filepath(filePath) { }

ShaderComponent CustomShaderBuilder::build()
{
	ShaderComponent shaderComponent;

	std::string& customShaderSource = Engine::get()->getShaderLoader()->readShader(m_filepath);

	ShadersInfo customShaders;
	Engine::get()->getShaderLoader()->parseGLSLShader(customShaderSource, customShaders);
	
	if(!customShaders.vertexCode.empty())
	{
		std::string& geomPassShaderSources = Engine::get()->getShaderLoader()->readShader(SGE_ROOT_DIR + "Resources/Engine/Shaders/PBR_GeomPassShader.glsl");

		ShadersInfo shaders;
		Engine::get()->getShaderLoader()->parseGLSLShader(geomPassShaderSources, shaders);

		std::string macro = "CUSTOM_SHADER";
		addMacro(shaders.vertexCode, macro);

		replaceDirective(shaders.vertexCode, "#custom_vert", customShaders.vertexCode);

		// build shader geom
		Shader* shader = new Shader();
		shader->BuildShaders(shaders);

		shaderComponent.m_vertexShader = shader;
	}

	if (!customShaders.fragmentCode.empty())
	{
		std::string& lightPassShaderSources = Engine::get()->getShaderLoader()->readShader(SGE_ROOT_DIR + "Resources/Engine/Shaders/PixelShader.glsl");

		ShadersInfo shaders;
		Engine::get()->getShaderLoader()->parseGLSLShader(lightPassShaderSources, shaders);

		std::string macro = "CUSTOM_SHADER";
		addMacro(shaders.fragmentCode, macro);

		replaceDirective(shaders.fragmentCode, "#custom_frag", customShaders.fragmentCode);

		// build shader light
		Shader* shader = new Shader();
		shader->BuildShaders(shaders);

		shaderComponent.m_fragmentShader = shader;
	}



	delete this;

	return shaderComponent;
}
