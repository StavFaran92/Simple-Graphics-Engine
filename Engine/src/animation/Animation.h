#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "memory/Asset.h"

class Bone;

struct MeshNodeData
{
	// convert from parent node space to this node space
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<MeshNodeData> children;
};

// Resource
class EngineAPI Animation : public Resource
{
public:
	struct LoadDescriptor : public ResourceLoadDescriptor
	{
		ResourceWrapper<Resource> loadResource() override {
			return Animation::load(*this);
		}
	};

	static ResourceWrapper<Animation> load(LoadDescriptor& desc);

	Animation();

	static ResourceWrapper<Animation> load(const std::string& fileLocation, LoadDescriptor desc = {});

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
public:
	using ResourceType = Animation;

	using Asset::Asset;

	static AssetHandle<AnimationAsset> import(const std::string& fileLocation, AssetCreateDescriptor desc = {});

	void save(const AssetRecord& aInfo) override;

protected:
	bool copyFiles(const std::string& fileLocation, AssetRecord& aInfo) override;
};