#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "memory/Asset.h"
#include "animation/AnimationLoader.h"

class Bone;

struct MeshNodeData
{
	// convert from parent node space to this node space
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<MeshNodeData> children;
};

// Asset IO Manager
struct AnimationAssetManager : public AssetManager
{
	bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) override;
	ResourceWrapper<Resource> load(AssetRecord& aInfo) override;
	void save(AssetHandle<Asset> asset, const AssetRecord& aInfo) override;
};

// Resource
class EngineAPI Animation : public Resource
{
public:
	Animation();

	void calculateFinalBoneMatrices(float currentTime, std::unordered_map<std::string, glm::mat4>& outFinalBoneMatrices);

	float getDuration() const;

	float getTicksPerSecond() const;

	void build(const std::string& name, float duration, float ticksPerSecond, MeshNodeData& rootNode, std::unordered_map<std::string, std::shared_ptr<Bone>>& bones);

	static bool preprocess(const std::string& path);	
private:
	void calculateFinalBoneMatricesHelper(const MeshNodeData& nodeData, glm::mat4 parentTransform, float currentTime, std::unordered_map<std::string, glm::mat4>& finalBoneMatrices);	

private:
	std::string m_name;
	MeshNodeData m_rootNode;
	std::unordered_map<std::string, std::shared_ptr<Bone>> m_bones;
	float m_duration = 0;
	float m_ticksPerSecond = 0;
};

// Asset
class EngineAPI AnimationAsset : public Asset
{
	using ResourceType = Animation;
public:
	static AssetHandle<AnimationAsset> import(const std::string& fileLocation, AnimationImportSettings settings = {});
};