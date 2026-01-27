#include "animation/Animation.h"

#include "animation/Bone.h"
#include "animation/AnimationLoader.h"

Animation::Animation()
{
}

ResourceWrapper<Animation> Animation::load(const std::string& fileLocation, AnimationLoadDescriptor desc)
{
	desc.origFilepath = fileLocation;
	return Engine::get()->getSubSystem<AnimationLoader>()->load(desc);
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

AssetHandle<AnimationAsset> AnimationAsset::import(const std::string& fileLocation, AssetCreateDescriptor desc)
{
	desc.aType = AssetType::ANIMATION;
	AnimationAsset* asset = new AnimationAsset(desc);
	return asset->importAsset(fileLocation).as<AnimationAsset>();
}

void AnimationAsset::save(const AssetRecord& aInfo)
{
	throw new std::runtime_error("Not yet implemented!");
}

bool AnimationAsset::copyFiles(const std::string& fileLocation, AssetRecord& aInfo)
{
	const std::string savedFilePath = aInfo.fullFilePath;
	return std::filesystem::copy_file(fileLocation, savedFilePath);
}

ResourceWrapper<Resource> AnimationLoadDescriptor::loadResource() {
	return Animation::load(*this);
}