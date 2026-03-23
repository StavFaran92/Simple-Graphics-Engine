#pragma once

#include <memory>
#include "memory/ResourceWrapper.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Exporter.hpp>

#include "core/Core.h"
#include "memory/Assets.h"
#include "memory/Asset.h"

#include "systems/SubSystem.h"

class Animation;
struct AnimationLoadDescriptor;

class EngineAPI AnimationLoader : public SubSystem
{
public:

	struct MeshNodeData
	{
		// convert from parent node space to this node space
		glm::mat4 transformation;
		std::string name;
		int childrenCount;
		std::vector<MeshNodeData> children;
	};

	struct AnimationInfo
	{
		std::string m_name;
		MeshNodeData m_rootNode;
		std::unordered_map<std::string, std::shared_ptr<Bone>> m_bones;
		float m_duration = 0;
		float m_ticksPerSecond = 0;
	};

	AnimationLoader();

	ResourceWrapper<Animation> load(const AnimationLoadDescriptor& aInfo);

	bool parseAnimation(const std::string& fileLocation, AnimationInfo& outAnimInfo);


private:
	Assimp::Importer m_importer;
};