#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <variant>
#include "memory/Asset.h"
#include "memory/AssetAliases.h"

#include "core/Core.h"

#include "glm/glm.hpp"

#include <nlohmann/json.hpp>
#include "render/ShadersInfo.h"

#include "MaterialProperty.h"

class TextureSamplerAsset;
class TerrainLayerAsset;

using json = nlohmann::json;

enum class ShaderOverride : int
{
	None,
	PBR,
	Pixel,
	Volume,
	PostProcess
};

struct EngineAPI ShaderCreateDescriptor : public ResourceBuildDescriptor
{
	ShaderOverride shaderOverride = ShaderOverride::None;

	std::string code = std::string(R"(

#vert

void vert(inout vec3 aPos, inout vec3 aNorm)
{
}

#frag

void frag(inout vec3 color, 
	inout vec3 normal, 
	inout float metallic, 
	inout float roughness, 
	inout float ao)
{      
    color = vec3(1.0f, 0.0f, 0.0f);
})");;
};

struct EngineAPI ShaderLoadDescriptor : public ResourceLoadDescriptor
{
	ShaderOverride shaderOverride = ShaderOverride::None;
};

extern EngineAPI const std::map<ShaderOverride, std::string> shaderOverrideToString;

struct ShadersInfo;
class Texture;
template<typename> class AssetTraits;

// Resource
class EngineAPI Shader : public Resource, std::enable_shared_from_this<Shader>
{
public:
	inline static const std::string ATTRIB_SHADER_OVERRIDE = "shader_override";

public:
	//static ShaderResourceRef create(std::string name, ShaderCreateDescriptor desc = {});

	static ShaderResourceRef load(const std::string& fileLocation, ShaderLoadDescriptor desc = {});

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

	void setTextureInShader(TextureResourceRef texture, const std::string& uniform, int slot);

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

	ShaderAsset() = default;
	ShaderAsset(const ShaderCreateDescriptor&);

	using Asset::Asset;

	void fillLoadDescriptor(ResourceLoadDescriptor& resourceLoadDesc) override;

	void serialize(nlohmann::json& j) const override;
	void deserialize(const nlohmann::json& j) override;

	ShaderOverride m_shaderOverride;
	bool m_isShaderOverride = false;
};
