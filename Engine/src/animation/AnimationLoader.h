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

#include "animation/Animation.h"

class Animation;
struct AnimationLoadDescriptor;

class EngineAPI AnimationLoader : public SubSystem
{
public:


	struct AnimationInfo
	{
		std::string m_name;
		std::vector<MeshNodeData> m_nodes;
		std::unordered_map<std::string, std::shared_ptr<Bone>> m_bones;
		float m_duration = 0;
		float m_ticksPerSecond = 0;
	};

	AnimationLoader();

	AnimationResourceRef load(const AnimationLoadDescriptor& aInfo);

	bool parseAnimation(const std::string& fileLocation, AnimationInfo& outAnimInfo);


private:
	Assimp::Importer m_importer;
};