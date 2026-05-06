#pragma once

#include <variant>
#include "glm/glm.hpp"
#include <memory>
#include <string>
#include <map>

#include "serialize/CerealHelpers.h"

class TextureSamplerAsset;
struct TerrainLayerAsset;

using Value = std::variant<
	float,
	glm::vec2,
	glm::vec3,
	glm::vec4,
	int,
	unsigned int,
	glm::mat3,
	glm::mat4,
	std::shared_ptr<TextureSamplerAsset>,
	std::shared_ptr<TerrainLayerAsset>>;

enum class MaterialPropertyType
{
	INT,
	UINT,
	FLOAT,
	VEC2,
	VEC3,
	VEC4,
	MAT3,
	MAT4,
	SAMPLER,

	TERRAIN_LAYER,

	// Must be last
	TOTAL_SIZE
};

struct PropertySchema {
	PropertySchema() : type(MaterialPropertyType::FLOAT) {};
	PropertySchema(MaterialPropertyType type)
		: type(type)
	{
	}

	std::string name;
	MaterialPropertyType type;
	std::string defaultValueRaw;
	float minValue = std::numeric_limits<float>::lowest();
	float maxValue = std::numeric_limits<float>::max();
	Value defaultValue;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(name);
		SERIALIZED_MEMBER(type);
		SERIALIZED_MEMBER(defaultValueRaw);
		SERIALIZED_MEMBER(minValue);
		SERIALIZED_MEMBER(maxValue);
		SERIALIZED_MEMBER(defaultValue);
	}
};

class MaterialLayout
{
public:
	const std::map<std::string, PropertySchema>& getAllProperties() const;
	void addProperty(const std::string& name, const PropertySchema& schema);
	bool hasProperty(const std::string& name) const;
	void clear();

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_propertySchemas);
	}

private:
	std::map<std::string, PropertySchema> m_propertySchemas;
};