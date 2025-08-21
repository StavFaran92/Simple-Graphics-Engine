#pragma once

#include <stdio.h>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <queue>
#include <variant>
#include "memory/Asset.h"

#include "core/Core.h"

#include "glm/glm.hpp"

enum class ShaderOverride : int
{
	PBR,
	Pixel,
	Volume,
	PostProcess
};

struct ShadersInfo;
template<typename> class Resource;
class Texture;

using Value = std::variant<float, glm::vec2, glm::vec3, glm::vec4, int, unsigned int, glm::mat3, glm::mat4>;

class EngineAPI Shader : public Asset, std::enable_shared_from_this<Shader>
{
public:

	void use();
	void release() const;

	inline unsigned int getID() const;

	void setUniformValue(const std::string& name, const Value& v);

	int getUniformLocation(const std::string& name);

	int getUniformBlockLocation(const std::string& name);

	void setModelMatrix(glm::mat4 model);
	void setViewMatrix(glm::mat4 view);
	void setProjectionMatrix(glm::mat4 projection);
	void setTime(float time);
	void init();

	void bindUniformBlockToBindPoint(const std::string& uniformBlockName, int bindPointIndex);

	void setTextureInShader(Resource<Texture> texture, const std::string& uniform, int slot);

	bool build();

	//void parseUniforms();

	ShaderOverride getShaderOverride() const { return shaderOverride; };

	//const std::unordered_map<std::string, int>& getAvailableUniforms() const

	bool recompile();

	const std::string& getSourceCode() const;

	static Resource<Shader> import(const std::string& filepath);
	static Resource<Shader> create(const std::string& filepath);
	static Resource<Shader> createOverrideShader(const std::string& name, const std::string& filepath, ShaderOverride shaderOverride, bool isTransient = false);
	static Resource<Shader> load(Resource<Shader> shader, const std::string& filepath, ShaderOverride shaderOverride);

	static ShaderOverride getShaderOverrideFromStr(const std::string& shaderOverride);
	static std::string getShaderOverrideAsStr(ShaderOverride shaderOverride);

	virtual ~Shader();

	/** Copy Constructor */
	Shader(const Shader& other) = default;

	/** Copy Assignemnt operator */
	Shader& operator=(const Shader & other) = default;

	Shader();

	/** Constructor */
	Shader(const std::string& glslFilePath);

	// Inherited via Asset
	void import(const std::string& fileLocation, const ImportSettings& settings) override;
	void load(AssetInfo aInfo) override;

protected:
	



	void clear();
	virtual void BuildShaders(const ShadersInfo& shader);
	uint32_t AddShader(const std::string& shaderCode, unsigned int shaderType);
	bool ValidateProgramLink();
	bool validateCompilation(const unsigned int& theShader, const unsigned int& shaderType);
private:
	friend class Context;
	inline void SetID(uint32_t id) { m_id = id; }

	void setFloat(const std::string& name, float v);
	void setFloat(const std::string& name, glm::vec2 v);
	void setFloat(const std::string& name, glm::vec3 v);
	void setFloat(const std::string& name, glm::vec4 v);
	void setInt(const std::string& name, int v);
	void setUInt(const std::string& name, unsigned int v);
	void setMat3(const std::string& name, const glm::mat3& v);
	void setMat4(const std::string& name, const glm::mat4& v);

	friend class CustomShaderBuilder;

protected:
	unsigned int m_id;
	std::unordered_map<std::string, int> m_uniformLocationCache;

	std::unordered_map<std::string, int> m_uniformBlockLocationCache;

	std::string m_glslFilePath;

	static uint32_t s_activeShader;

	std::unordered_map<std::string, Value> m_delayedProperties;

	ShaderOverride shaderOverride;
	bool m_isShaderOverride = false;

	//std::unordered_map<std::string, Value> m_uniformProperties;
	//std::unordered_map<std::string, Resource<Texture>> m_textures;

	std::string m_sourceCode;
	std::string origSourceCode;
};
