#include "animation/Animation.h"

#include "animation/Bone.h"
#include "animation/AnimationLoader.h"

#include <filesystem>

template<>
struct AssetTraits<Animation>
{
	static bool copyFiles(const std::string& fileLocation, AssetInfo& aInfo)
	{
		return Engine::get()->getSubSystem<AnimationLoader>()->copyFileToResourceFolder(fileLocation, aInfo);
	}

	static void convertAssetLoadParamsToAssetInfo(const std::string& fileLocation, const BaseAssetParameters& params, AssetInfo& aInfo)
	{
		// Data extract
		if (params.name.empty())
		{
			aInfo.name = std::filesystem::path(fileLocation).filename().stem().string();
		}
		else
		{
			aInfo.name = params.name;
		}

		aInfo.aType = AssetType::ANIMATION;

		const std::string relativeFilepath = "/" + aInfo.name + ".dae";
		aInfo.filePath = relativeFilepath;
		aInfo.origFilePath = fileLocation;
	}

	static Resource<Animation> load(AssetInfo& aInfo)
	{
		return Engine::get()->getSubSystem<AnimationLoader>()->load(aInfo);
	}
};

static AssetFnRegister<AssetType::ANIMATION> assetRegister(AssetTraits<Animation>::load);

Animation::Animation()
{
}

void Animation::calculateFinalBoneMatricesHelper(const MeshNodeData& nodeData, glm::mat4 parentTransform, float currentTime, std::unordered_map<std::string, glm::mat4>& finalBoneMatrices)
{
	/*
		We look at the vertex in mesh space at the beginning, 
		now we want to look at it relative to the bone it relates to
		so we create the invert-bind matrix that takes a vertex in mesh space and converts it to bone space (different for each bone)
		for example if a bone is rotated to do side for example 45 degrees, then we need to rotate our view by 45
		degrees to view the vertex as if it relative to the bone (so the bone center is at (0,0))
		so we take the the transofrmation of the bone (e.g. rotated 45 deg)
		and we multiply be parent
		so each matrix holds the data of "how to get a vertex from mesh space into tthis bone's space"
	*/

	/* 
	We essentialy wish to go from Bone "Bind" space to Pose space,
	let Bind space be S(x) and final pose space be T(x)
	then to take Bone from one to the other we need S^-1(x)*T(x)
	by multiplying by parentTransform we generate T(x),
	and S^-1(x) is given to us by Assimp,
	in the case it was not provided we could have created it recursivly by inversing all the 
	transformations until the bone is bind space
	more can be explained on the matter in 
	https://www.youtube.com/watch?v=cieheqt7eqc
	at 11:57
	*/

	std::string nodeName = nodeData.name;
	glm::mat4 nodeTransform = nodeData.transformation;
	glm::mat4 globalTransformation = parentTransform * nodeTransform; //convert bone transform from bone space into parent space (eventually into animated mesh space)

	if (m_bones.find(nodeName) != m_bones.end())
	{
		auto bone = m_bones[nodeName];
		bone->update(currentTime);
		nodeTransform = bone->getLocalTransform();
		globalTransformation = parentTransform * nodeTransform;
	}

	finalBoneMatrices[nodeName] = globalTransformation;

	for (int i = 0; i < nodeData.childrenCount; i++)
	{
		calculateFinalBoneMatricesHelper(nodeData.children[i], globalTransformation, currentTime, finalBoneMatrices);
	}


}

void Animation::calculateFinalBoneMatrices(float currentTime, std::unordered_map<std::string, glm::mat4>& outFinalBoneMatrices)
{
	return calculateFinalBoneMatricesHelper(m_rootNode, glm::mat4(1.0), currentTime, outFinalBoneMatrices);
}

float Animation::getDuration() const
{
	return m_duration;
}

float Animation::getTicksPerSecond() const
{
	return m_ticksPerSecond;
}

void Animation::build(const std::string& name, float duration, float ticksPerSecond, MeshNodeData& rootNode, std::unordered_map<std::string, std::shared_ptr<Bone>>& bones)
{
	m_name = name;
	m_duration = duration;
	m_ticksPerSecond = ticksPerSecond;
	m_rootNode = rootNode;
	m_bones = bones;
}

bool Animation::preprocess(const std::string& path)
{
	return true;
}

//#include "animation/AnimationLoader.h"
//void Animation::load(UUID uid, const std::string& path)
//{
//	Engine::get()->getSubSystem<AnimationLoader>()->load(path, Resource<Animation>(uid));
//
//}

Resource<Animation> Animation::import(const std::string& fileLocation, const AnimationImportSettings& settings)
{
	return AssetLoader<Animation>::import(fileLocation, settings);
}

Resource<Animation> Animation::loadTransient(const std::string& fileLocation, const AnimationImportSettings& settings)
{
	return AssetLoader<Animation>::loadTransient(fileLocation, settings);
}
