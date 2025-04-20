#include "Shader.h"

#include <regex>

#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include "Utils.h"
#include "Logger.h"
#include "ShaderLoader.h"
#include "Engine.h"
#include "Texture.h"
#include "Resource.h"
#include "Factory.h"
#include "Assets.h"
#include "CommonTextures.h"

enum ShaderType
{
	FRAGMENT_SHADER = 0x8B30,
	VERTEX_SHADER = 0x8B31,
	GEOMETRY_SHADER = 0x8DD9,
	TESS_CONTROL_SHADER = 0x8E88,
	TESS_EVALUATION_SHADER = 0x8E87


};

int extractErrorLineNumberFromLog(const std::string& log)
{
	// Try matching 0(<number>)
	size_t pos0 = log.find("0(");
	if (pos0 != std::string::npos)
	{
		size_t openParen = log.find('(', pos0);
		size_t closeParen = log.find(')', openParen);
		if (openParen != std::string::npos && closeParen != std::string::npos)
		{
			std::string numStr = log.substr(openParen + 1, closeParen - openParen - 1);
			try { return std::stoi(numStr); }
			catch (...) { return -1; }
		}
	}

	// Try matching 0:<number> (colon format)
	size_t colon = log.find("0:");
	if (colon != std::string::npos)
	{
		size_t start = colon + 2;
		size_t end = log.find(':', start);
		std::string numStr = (end == std::string::npos) ? log.substr(start) : log.substr(start, end - start);
		try { return std::stoi(numStr); }
		catch (...) { return -1; }
	}

	return -1; // failed to find
}

std::string shaderTypeToStr(ShaderType sType)
{
	switch (sType)
	{
	case ShaderType::FRAGMENT_SHADER: return "Fragment Shader";
	case ShaderType::VERTEX_SHADER: return "Vertex Shader";
	case ShaderType::GEOMETRY_SHADER: return "Geometry Shader";
	case ShaderType::TESS_CONTROL_SHADER: return "Tesselation Control Shader";
	case ShaderType::TESS_EVALUATION_SHADER: return "Tesselation Eval Shader";
	}
}

uint32_t Shader::s_activeShader = 0;

Shader::Shader()
{
}

Shader::Shader(const std::string& glslFilePath) :
	m_id(0),
	m_glslFilePath(glslFilePath)
{
	recompile();
}

void Shader::init()
{

}

void Shader::bindUniformBlockToBindPoint(const std::string& uniformBlockName, int bindPointIndex)
{
	int uniformBlockIndex = getUniformBlockLocation(uniformBlockName);
	if (uniformBlockIndex < 0) return;
	glUniformBlockBinding(m_id, uniformBlockIndex, bindPointIndex);
}

void Shader::setTextureInShader(Resource<Texture> texture, const std::string& uniform, int slot)
{
	texture.get()->setSlot(slot);
	texture.get()->bind();
	setUniformValue(uniform, slot);
}

//void Shader::parseUniforms()
//{
//	std::regex uniformRegex(R"(uniform\s+(\w+)\s+(\w+)\s*;)");
//	std::smatch match;
//	std::string::const_iterator searchStart(m_sourceCode.cbegin());
//
//	auto& uniformProperties = m_uniformProperties;
//
//	while (std::regex_search(searchStart, m_sourceCode.cend(), match, uniformRegex)) {
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
//			m_textures[name] = Engine::get()->getCommonTextures()->getTexture(CommonTextures::TextureType::WHITE_1X1);
//		}
//
//		searchStart = match.suffix().first;
//	}
//}

void Shader::BuildShaders(const ShadersInfo& shaderCode)
{
	//Create a new shader program
	m_id = glCreateProgram();

	// Validate shader program creation
	if (!m_id)
	{
		logError("Error creating program");
		return;
	}

	// Create and attach vertex shader to program
	GLuint vertexShader = AddShader(shaderCode.vertexCode, GL_VERTEX_SHADER);
	glAttachShader(m_id, vertexShader);

	// Create and attach fragment shader to program
	GLuint fragShader = AddShader(shaderCode.fragmentCode, GL_FRAGMENT_SHADER);
	glAttachShader(m_id, fragShader);

	// Create and attach geometry shader to program if needed
	GLuint geometryShader = 0;
	if (!shaderCode.geometryCode.empty())
	{
		geometryShader = AddShader(shaderCode.geometryCode, GL_GEOMETRY_SHADER);
		glAttachShader(m_id, geometryShader);
	}

	// Create and attach tesselation control shader to program if needed
	GLuint tessCtrlShader = 0;
	if (!shaderCode.tessControlCode.empty())
	{
		tessCtrlShader = AddShader(shaderCode.tessControlCode, GL_TESS_CONTROL_SHADER);
		glAttachShader(m_id, tessCtrlShader);
	}

	// Create and attach tesselation evaluation shader to program if needed
	GLuint tessEvalShader = 0;
	if (!shaderCode.tessEvaluationCode.empty())
	{
		tessEvalShader = AddShader(shaderCode.tessEvaluationCode, GL_TESS_EVALUATION_SHADER);
		glAttachShader(m_id, tessEvalShader);
	}

	// Link shader program
	glLinkProgram(m_id);

	// Validate shader program link
	ValidateProgramLink();

	GLint activeUniforms = 0;
	glGetProgramiv(m_id, GL_ACTIVE_UNIFORMS, &activeUniforms);

	for (GLint i = 0; i < activeUniforms; ++i)
	{
		char name[256];
		GLsizei length;
		GLint size;
		GLenum type;
		glGetActiveUniform(m_id, i, sizeof(name), &length, &size, &type, name);
		GLint location = glGetUniformLocation(m_id, name);
		m_uniformLocationCache[name] = location;
	}

	// Delete shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);

	if (geometryShader)
		glDeleteShader(geometryShader);

	if (tessCtrlShader)
		glDeleteShader(tessCtrlShader);

	if (tessEvalShader)
		glDeleteShader(tessEvalShader);

	//parseUniforms();
}

bool Shader::ValidateProgramLink()
{
	GLint result = 0;
	glGetProgramiv(m_id, GL_LINK_STATUS, &result);
	if (!result)
	{
		GLchar eLog[1024] = { 0 };
		glGetProgramInfoLog(m_id, sizeof(eLog), NULL, eLog);
		logError("Error linking program: {}", eLog);
		return false;
	}

	glValidateProgram(m_id);

	glGetProgramiv(m_id, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		GLchar eLog[1024] = { 0 };
		glGetProgramInfoLog(m_id, sizeof(eLog), NULL, eLog);
		logError("Error validating program: {}", eLog);
		return false;
	}
	return true;
}

void Shader::use()
{
	if (s_activeShader == m_id) return;

	s_activeShader = m_id;

	glUseProgram(m_id);

	for (auto it = m_delayedProperties.begin(); it != m_delayedProperties.end(); ++it)
	{
		setUniformValue(it->first, it->second);
	}

	m_delayedProperties.clear();
}

void Shader::release() const
{
	s_activeShader = 0;

	glUseProgram(0);
}

void Shader::clear()
{
	if (m_id != 0) {
		glDeleteProgram(m_id);
		m_id = 0;
	}
}

uint32_t Shader::AddShader(const std::string& shaderCode, unsigned int shaderType)
{
	GLuint shader = glCreateShader(shaderType);

	const GLchar* code[] = { shaderCode.c_str() };
	const GLint length[] = { shaderCode.size() };

	glShaderSource(shader, 1, code, length);
	glCompileShader(shader);

	if (!validateCompilation(shader, shaderType, shaderCode))
	{
		return -1;
	}

	return shader;
}

bool Shader::validateCompilation(unsigned int shaderID, unsigned int shaderType, const std::string& shaderCode)
{
	GLint result = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		GLchar eLog[1024] = { 0 };
		glGetShaderInfoLog(shaderID, 1024, NULL, eLog);

		int lineNumber = extractErrorLineNumberFromLog(eLog);

		// Now split shaderCode by lines
		std::vector<std::string> lines;
		std::stringstream ss(shaderCode);
		std::string line;
		while (std::getline(ss, line))
		{
			lines.push_back(line);
		}

		lineNumber -= 2; // offset

		// Print error and line
		if (lineNumber >= 0 && lineNumber < lines.size())
		{
			logError("Error compiling shader {}, shader type: {}, {}. line: {} : {}",
				std::to_string(shaderID),
				shaderTypeToStr((ShaderType)shaderType),
				eLog,
				lineNumber,
				lines[lineNumber]);
		}

		
		return false;
	}
	return true;
}

//int Shader::getUniformBlockLocation(const std::string& name)
//{
//	if (m_uniformBlockLocationCache.find(name) != m_uniformBlockLocationCache.end())
//		return m_uniformBlockLocationCache[name];
//
//	int location = glGetUniformBlockIndex(m_id, name.c_str());
//
//	if (location == -1) {
//		//logWarning("Uniform {} doesn't exists!", name.c_str());
//		return -1;
//	}
//
//	m_uniformBlockLocationCache[name] = location;
//
//	return location;
//}

int Shader::getUniformLocation(const std::string& name)
{
	if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end())
		return m_uniformLocationCache[name];

	return -1;
}

int Shader::getUniformBlockLocation(const std::string& name)
{
	if (m_uniformBlockLocationCache.find(name) != m_uniformBlockLocationCache.end())
		return m_uniformBlockLocationCache[name];

	int location = glGetUniformBlockIndex(m_id, name.c_str());

	if (location == -1) 
	{
		return -1;
	}

	m_uniformBlockLocationCache[name] = location;

	return location;
}

inline unsigned int Shader::getID() const
{
	return m_id;
}

void Shader::setUniformValue(const std::string& name, const Value& v)
{
	if (s_activeShader != m_id)
	{
		m_delayedProperties[name] = v;
		return;
	}

	if (m_uniformLocationCache.find(name) == m_uniformLocationCache.end())
	{
		return;
	}

	std::visit([this, &name](auto&& arg) {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, float>)
			this->setFloat(name, arg);
		else if constexpr (std::is_same_v<T, glm::vec2>)
			this->setFloat(name, arg);
		else if constexpr (std::is_same_v<T, glm::vec3>)
			this->setFloat(name, arg);
		else if constexpr (std::is_same_v<T, glm::vec4>)
			this->setFloat(name, arg);
		else if constexpr (std::is_same_v<T, int>)
			this->setInt(name, arg);
		else if constexpr (std::is_same_v<T, unsigned int>)
			this->setUInt(name, arg);
		else if constexpr (std::is_same_v<T, glm::mat3>)
			this->setMat3(name, arg);
		else if constexpr (std::is_same_v<T, glm::mat4>)
			this->setMat4(name, arg);
		}, v);
}

void Shader::setFloat(const std::string& name, float v)
{
	glUniform1f(getUniformLocation(name), v);
}

void Shader::setFloat(const std::string& name, glm::vec2 v)
{
	glUniform2f(getUniformLocation(name), v.x, v.y);
}

void Shader::setFloat(const std::string& name, glm::vec3 v)
{
	glUniform3f(getUniformLocation(name), v.x, v.y, v.z);
}

void Shader::setFloat(const std::string& name, glm::vec4 v)
{
	glUniform4f(getUniformLocation(name), v.x, v.y, v.z, v.w);
}

void Shader::setInt(const std::string& name, int v)
{
	glUniform1i(getUniformLocation(name), v);
}

void Shader::setUInt(const std::string& name, unsigned int v)
{
	glUniform1ui(getUniformLocation(name), v);
}

void Shader::setMat3(const std::string& name, const glm::mat3& v)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(v));
}

void Shader::setMat4(const std::string& name, const glm::mat4& v)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(v));
}

void Shader::setModelMatrix(glm::mat4 model)
{
	setMat4("model", model);
}

void Shader::setViewMatrix(glm::mat4 view)
{
	setMat4("view", view);
}

void Shader::setProjectionMatrix(glm::mat4 projection)
{
	setMat4("projection", projection);
}

void Shader::setTime(float time)
{
	setFloat("time", time);
}

Resource<Shader> Shader::create(const std::string& filepath)
{
	return Factory<Shader>::create(filepath);
}

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

void replaceDirective(std::string& source, const std::string& directive, std::string replacement)
{
	size_t pos = source.find(directive);
	if (pos != std::string::npos)
	{
		source.replace(pos, directive.length(), replacement);
	}
}

Resource<Shader> Shader::createOverrideShader(const std::string& name, const std::string& filepath, ShaderOverride shaderOverride)
{
	Resource<Shader> shader = Factory<Shader>::create();
	shader->m_isShaderOverride = true;
	shader->shaderOverride = shaderOverride;
	shader->m_glslFilePath = filepath;
	shader->recompile();

	AssetInfo aInfo;
	aInfo.uuid = shader.getUID();
	aInfo.origFilePath = filepath;
	aInfo.aType = AssetType::SHADER;
	aInfo.name = name;
	aInfo.attributes["shader_override"] = getShaderOverrideAsStr(shaderOverride);
	Engine::get()->getSubSystem<Assets>()->importAsset(aInfo);

	return shader;
}

Resource<Shader> Shader::load(Resource<Shader> shader, const std::string& filepath, ShaderOverride shaderOverride)
{
	shader->m_isShaderOverride = true;
	shader->shaderOverride = shaderOverride;
	shader->m_glslFilePath = filepath;
	shader->recompile();

	return shader;
}

ShaderOverride Shader::getShaderOverrideFromStr(const std::string& shaderOverride)
{
	if (shaderOverride == "PBR") return ShaderOverride::PBR;
	if (shaderOverride == "Pixel") return ShaderOverride::Pixel;
	return ShaderOverride::PBR;
}

std::string Shader::getShaderOverrideAsStr(ShaderOverride shaderOverride)
{
	switch (shaderOverride)
	{
	case ShaderOverride::PBR:
		return "PBR";
	case ShaderOverride::Pixel:
		return "Pixel";
	}
	return "N/A";
}

void embeddOverrideShaderInUberShader(ShadersInfo& shaderOverrideInfo, ShaderOverride shaderOverride)
{
	if (!shaderOverrideInfo.vertexCode.empty() || !shaderOverrideInfo.fragmentCode.empty())
	{
		std::string shaderPath;
		if (shaderOverride == ShaderOverride::PBR)
		{
			shaderPath = SGE_ROOT_DIR + "Resources/Engine/Shaders/PBRShader.glsl";
		}
		else if (shaderOverride == ShaderOverride::Pixel)
		{
			shaderPath = SGE_ROOT_DIR + "Resources/Engine/Shaders/PixelShader.glsl";
		}

		std::string& pixelShaderSources = Engine::get()->getShaderLoader()->readShader(shaderPath);

		ShadersInfo pixelShaderInfo;
		Engine::get()->getShaderLoader()->parseGLSLShader(pixelShaderSources, pixelShaderInfo);

		std::string macro = "CUSTOM_SHADER";

		if (!pixelShaderInfo.vertexCode.empty())
		{
			addMacro(pixelShaderInfo.vertexCode, macro);
			replaceDirective(pixelShaderInfo.vertexCode, "#custom_vert", shaderOverrideInfo.vertexCode.c_str());
		}

		if (!pixelShaderInfo.fragmentCode.empty())
		{
			addMacro(pixelShaderInfo.fragmentCode, macro);
			replaceDirective(pixelShaderInfo.fragmentCode, "#custom_frag", shaderOverrideInfo.fragmentCode.c_str()); // I have no idea why but casting it to cstring fix the overwrite issue
		}

		shaderOverrideInfo = pixelShaderInfo;
	}

	
}

bool Shader::recompile()
{
	ShadersInfo shadersInfo;

	std::string& fullShaderCode = Engine::get()->getShaderLoader()->readShader(m_glslFilePath);
	Engine::get()->getShaderLoader()->parseGLSLShader(fullShaderCode, shadersInfo);

	if (m_isShaderOverride)
	{
		embeddOverrideShaderInUberShader(shadersInfo, shaderOverride);
	}
	
	BuildShaders(shadersInfo);

	m_sourceCode = fullShaderCode;

	return true;
}

const std::string& Shader::getSourceCode() const
{
	return m_sourceCode;
}

Resource<Shader> Shader::import(const std::string& filepath)
{
	Resource<Shader> shader = Factory<Shader>::create(filepath);

	AssetInfo aInfo;
	aInfo.uuid = shader.getUID();
	aInfo.origFilePath = filepath;
	aInfo.aType = AssetType::SHADER;
	Engine::get()->getSubSystem<Assets>()->importAsset(aInfo); // this is a hack, we should load the copied shader.
	return shader;
}

Shader::~Shader() {
	logInfo(__FUNCTION__);
	// TODO fix, this is currently a gpu memory leak
	//clear();
}