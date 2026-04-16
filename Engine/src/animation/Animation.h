#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "memory/AssetAliases.h"
#include "memory/Asset.h"

class Bone;
struct AnimationLoadDescriptor;

struct MeshNodeData
{
	// convert from parent node space to this node space
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<int> children;
};

struct AnimationData
{
	std::string name;
	std::unordered_map<std::string, std::shared_ptr<Bone>> bones;
	float duration = 0;
	float ticksPerSecond = 0;
	std::vector<MeshNodeData> nodes;
};

struct EngineAPI AnimationLoadDescriptor : public ResourceLoadDescriptor
{
};

struct EngineAPI AnimationCreateDescriptor : public ResourceBuildDescriptor
{
	AnimationData data;
};

// Resource
class EngineAPI Animation : public Resource
{
public:
	static AnimationResourceRef load(const std::string& fileLocation, AnimationLoadDescriptor desc = {});

	Animation();

	void calculateFinalBoneMatrices(float currentTime, std::unordered_map<std::string, glm::mat4>& outFinalBoneMatrices);

	float getDuration() const;

	float getTicksPerSecond() const;

	void build(const AnimationData& animationData);

	const AnimationData& getData() const;

	static bool preprocess(const std::string& path);	
private:
	void calculateFinalBoneMatricesHelper(const MeshNodeData& nodeData, glm::mat4 parentTransform, float currentTime, std::unordered_map<std::string, glm::mat4>& finalBoneMatrices);	

private:
	AnimationData m_data;
};

// Asset
class EngineAPI AnimationAsset : public Asset
{
public:
	using ResourceType = Animation;

	AnimationAsset() = default;

	// Inherited via Asset
	void serialize(nlohmann::json& j) const override;
	void deserialize(const nlohmann::json& j) override;
};
