#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "UUID.h"

class Bone;

struct MeshNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<MeshNodeData> children;
};

class Animation
{
public:
	Animation();
	void calculateFinalBoneMatrices(float currentTime, std::unordered_map<std::string, glm::mat4>& outFinalBoneMatrices);
	float getDuration() const;
	float getTicksPerSecond() const;
	void build(const std::string& name, float duration, float ticksPerSecond, MeshNodeData& rootNode, std::unordered_map<std::string, std::shared_ptr<Bone>>& bones);

	static bool preprocess(const std::string& path);
	static void load(UUID uid, const std::string& path);
private:
	void calculateFinalBoneMatricesHelper(const MeshNodeData& nodeData, glm::mat4 parentTransform, float currentTime, std::unordered_map<std::string, glm::mat4>& finalBoneMatrices);

	

private:
	std::string m_name;
	MeshNodeData m_rootNode;
	std::unordered_map<std::string, std::shared_ptr<Bone>> m_bones;
	float m_duration = 0;
	float m_ticksPerSecond = 0;
};