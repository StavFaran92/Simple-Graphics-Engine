#include "Shader.h"

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

uint32_t Shader::s_activeShader = 0;

Shader::Shader()
{
}

Shader::Shader(const std::string& glslFilePath) :
	m_id(0),
	m_glslFilePath(glslFilePath)
{
	std::string& fullShaderCode = Engine::get()->getShaderLoader()->readShader(glslFilePath);

	ShadersInfo shaders;
	Engine::get()->getShaderLoader()->parseGLSLShader(fullShaderCode, shaders);

	// Build shaders
	BuildShaders(shaders);
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

Resource<Shader> Shader::import(const std::string& filepath)
{
	Resource<Shader> shader = Factory<Shader>::create(filepath);
	Engine::get()->getSubSystem<Assets>()->importAsset(shader.getUID(), filepath, AssetType::SHADER); // this is a hack, we should load the copied shader.
	return shader;
}

Shader::~Shader() {
	logInfo(__FUNCTION__);
	// TODO fix, this is currently a gpu memory leak
	//clear();
}