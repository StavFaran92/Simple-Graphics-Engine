#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <variant>
#include "memory/Asset.h"

#include "core/Core.h"

#include "glm/glm.hpp"

#include <nlohmann/json.hpp>
#include "render/ShadersInfo.h"

using json = nlohmann::json;

enum class ShaderOverride : int
{
	None,
	PBR,
	Pixel,
	Volume,
	PostProcess
};

struct ShaderLoadDescriptor : public ResourceLoadDescriptor
{
	ShaderLoadDescriptor();

	ShaderOverride shaderOverride = ShaderOverride::None;

	json fillParams() const override
	{
		return *this;
	}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE(ShaderLoadDescriptor,
		shaderOverride
	);
};

extern EngineAPI const std::map<ShaderOverride, std::string> shaderOverrideToString;

struct ShadersInfo;
template<typename> class ResourceWrapper;
class Texture;
template<typename> class AssetTraits;

using Value = std::variant<float, glm::vec2, glm::vec3, glm::vec4, int, unsigned int, glm::mat3, glm::mat4>;

// Resource
class EngineAPI Shader : public Resource, std::enable_shared_from_this<Shader>
{
public:
	inline static const std::string ATTRIB_SHADER_OVERRIDE = "shader_override";

public:
	static ResourceWrapper<Shader> createOverrideShader(const std::string& filepath, ShaderOverride shaderOverride, bool isEngineOwned = false);

	static ResourceWrapper<Shader> load(const std::string& fileLocation, ShaderLoadDescriptor desc = {});

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

	void setTextureInShader(ResourceWrapper<Texture> texture, const std::string& uniform, int slot);

	bool build();

	ShaderOverride getShaderOverride() const { return shaderOverride; };

	bool recompile();

	const std::string& getSourceCode() const;

	const ShadersInfo& getShadersInfo() const;

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

	std::string m_sourceCode;
	std::string origSourceCode;

	ShadersInfo m_shadersInfo;
};

// Asset
class EngineAPI ShaderAsset : public Asset
{
public:
	using ResourceType = Shader;

	using Asset::Asset;

	static AssetHandle<ShaderAsset> import(const std::string& fileLocation, AssetCreateDescriptor desc = {});

	static AssetHandle<ShaderAsset> create(AssetCreateDescriptor desc);

	void save(const AssetRecord& aInfo) override;

	

protected:
	bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) override;
};