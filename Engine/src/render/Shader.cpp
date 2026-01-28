#include "render/Shader.h"

#include <regex>

#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>

#include "utils/Utils.h"
#include "core/Logger.h"
#include "render/ShaderLoader.h"
#include "core/Engine.h"
#include "texture/Texture.h"
#include "memory/ResourceWrapper.h"
#include "core/Factory.h"
#include "memory/Assets.h"
#include "memory/AssetLoader.h"


#include <filesystem>

const  std::map<ShaderOverride, std::string> shaderOverrideToString
{
	{ ShaderOverride::None,			"N/A" },
	{ ShaderOverride::PBR,		"PBR" },
	{ ShaderOverride::Pixel,		"Pixel" },
	{ ShaderOverride::Volume,		"Volume" },
	{ ShaderOverride::PostProcess,		"PostProcess" },
};

bool ShaderAsset::copyFiles(const std::string& fileLocation, AssetRecord& aInfo)
{
	return std::filesystem::copy_file(fileLocation, aInfo.fullFilePath, std::filesystem::copy_options::overwrite_existing);
}

void ShaderAsset::save(const AssetRecord& aInfo)
{
	std::filesystem::copy_file(aInfo.sourcePath, aInfo.fullFilePath, std::filesystem::copy_options::overwrite_existing);
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

void Shader::setTextureInShader(ResourceWrapper<Texture> texture, const std::string& uniform, int slot)
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
	GLuint vertexShader = 0;
	if (!shaderCode.vertexCode.empty())
	{
		vertexShader = AddShader(shaderCode.vertexCode, GL_VERTEX_SHADER);
		glAttachShader(m_id, vertexShader);
	}

	// Create and attach fragment shader to program
	GLuint fragShader = 0;
	if (!shaderCode.fragmentCode.empty())
	{
		fragShader = AddShader(shaderCode.fragmentCode, GL_FRAGMENT_SHADER);
		glAttachShader(m_id, fragShader);
	}

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

	// Create and attach compute shader
	GLuint computeShader = 0;
	if (!shaderCode.computeCode.empty())
	{
		computeShader = AddShader(shaderCode.computeCode, GL_COMPUTE_SHADER);
		glAttachShader(m_id, computeShader);
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
		GLsizei length = 0;
		GLint size = 0;
		GLenum type = 0;

		glGetActiveUniform(m_id, i, sizeof(name), &length, &size, &type, name);

		std::string baseName(name);

		// Check if it's an array element: ends with [0]
		bool isArray = false;
		if (baseName.size() >= 3 && baseName.substr(baseName.size() - 3) == "[0]") {
			isArray = true;
			baseName = baseName.substr(0, baseName.size() - 3); // strip "[0]"
		}

		// Handle arrays
		if (isArray) {
			for (int j = 0; j < size; ++j) {
				std::string elem = baseName + "[" + std::to_string(j) + "]";
				GLint loc = glGetUniformLocation(m_id, elem.c_str());
				if (loc != -1)
					m_uniformLocationCache[elem] = loc;
			}
		}
		else {
			GLint loc = glGetUniformLocation(m_id, baseName.c_str());
			if (loc != -1)
				m_uniformLocationCache[baseName] = loc;
		}
	}

	// Delete 
	if(vertexShader)
		glDeleteShader(vertexShader);

	if(fragShader)
		glDeleteShader(fragShader);

	if (geometryShader)
		glDeleteShader(geometryShader);

	if (tessCtrlShader)
		glDeleteShader(tessCtrlShader);

	if (tessEvalShader)
		glDeleteShader(tessEvalShader);

	if (computeShader)
		glDeleteShader(computeShader);

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

	if (!validateCompilation(shader, shaderType))
		return -1;

	return shader;
}

bool Shader::validateCompilation(const unsigned int& shader, const unsigned int& shaderType)
{
	GLint result = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		GLchar eLog[1024] = { 0 };
		glGetShaderInfoLog(shader, 1024, NULL, eLog);
		logError("Error compiling the {} shader: {}", shaderType, eLog);
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
		//logWarning("Could not find " + name + " in shader.");
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
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(v));
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

ResourceWrapper<Shader> Shader::createOverrideShader(const std::string& filepath, ShaderOverride shaderOverride, bool isEngineOwned)
{
	ResourceWrapper<Shader> shader = Factory<Shader>::create();

	shader->m_isShaderOverride = true;
	shader->shaderOverride = shaderOverride;
	shader->m_glslFilePath = filepath;
	shader->recompile();

	return shader;
}

ResourceWrapper<Shader> Shader::load(const std::string& fileLocation, ShaderLoadDescriptor desc)
{
	ShaderOverride shaderOverride = desc.shaderOverride;

	std::string filepath = fileLocation;

	ResourceWrapper<Shader> shader = Factory<Shader>::create();
	shader->m_isShaderOverride = shaderOverride != ShaderOverride::None;
	shader->shaderOverride = shaderOverride;
	shader->m_glslFilePath = filepath;
	shader->recompile();

	return shader;
}

//Resource<Shader> Shader::load(Resource<Shader> shader, const std::string& filepath, ShaderOverride shaderOverride)
//{
//	shader->m_isShaderOverride = true;
//	shader->shaderOverride = shaderOverride;
//	shader->m_glslFilePath = filepath;
//	shader->recompile();
//
//	return shader;
//}

ShaderOverride Shader::getShaderOverrideFromStr(const std::string& shaderOverride)
{
	for (const auto& [key, value] : shaderOverrideToString)
	{
		if (value == shaderOverride)
			return key;
	}

	logWarning("Invalid shader override string: {}", shaderOverride);

	return ShaderOverride::None;
}

std::string Shader::getShaderOverrideAsStr(ShaderOverride shaderOverride)
{
	auto it = shaderOverrideToString.find(shaderOverride);
	if (it != shaderOverrideToString.end())
		return it->second;

	return shaderOverrideToString.at(ShaderOverride::None);
}

void embeddOverrideShaderInUberShader(ShadersInfo& shaderOverrideInfo, ShaderOverride shaderOverride)
{
	if (!shaderOverrideInfo.vertexCode.empty() || !shaderOverrideInfo.fragmentCode.empty())
	{
		std::string shaderPath;
		if (shaderOverride == ShaderOverride::PBR)
		{
			shaderPath = SGE_ROOT_DIR "Resources/Engine/Shaders/PBRShader.glsl";
		}
		else if (shaderOverride == ShaderOverride::Pixel)
		{
			shaderPath = SGE_ROOT_DIR "Resources/Engine/Shaders/PixelShader.glsl";
		}
		else if (shaderOverride == ShaderOverride::PostProcess)
		{
			shaderPath = SGE_ROOT_DIR "Resources/Engine/Shaders/PostProcessShader.glsl";
		}
		else if (shaderOverride == ShaderOverride::Volume)
		{
			shaderPath = SGE_ROOT_DIR "Resources/Engine/Shaders/VolumeShader.glsl";
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
	m_shadersInfo = shadersInfo;

	return true;
}

const std::string& Shader::getSourceCode() const
{
	return m_sourceCode;
}

const ShadersInfo& Shader::getShadersInfo() const
{
	return m_shadersInfo;
}

Shader::~Shader() {
	logInfo(__FUNCTION__);
	// TODO fix, this is currently a gpu memory leak
	//clear();
}

AssetHandle<ShaderAsset> ShaderAsset::import(const std::string& fileLocation, AssetCreateDescriptor desc)
{
	desc.aType = AssetType::SHADER;
	ShaderAsset* asset = new ShaderAsset(desc);
	return asset->importAsset(fileLocation).as<ShaderAsset>();
}

AssetHandle<ShaderAsset> ShaderAsset::create(const ResourceWrapper<Shader>& shader, AssetCreateDescriptor desc)
{
	desc.aType = AssetType::SHADER;
	ShaderAsset* asset = new ShaderAsset(desc);
	return asset->createAsset(shader).as<ShaderAsset>();
}

ResourceWrapper<Resource> ShaderLoadDescriptor::loadResource() 
{
	return Shader::load(sourcePath, *this);
}