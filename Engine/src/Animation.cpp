#include "Animation.h"

#include "Bone.h"

Animation::Animation()
{
}

void Animation::calculateFinalBoneMatricesHelper(const MeshNodeData& nodeData, glm::mat4 parentTransform, float currentTime, std::unordered_map<std::string, glm::mat4>& finalBoneMatrices)
{
	std::string nodeName = nodeData.name;
	glm::mat4 nodeTransform = nodeData.transformation;

	//auto iter = m_bones.find(nodeName);
	//if (iter != m_bones.end())
	//{
	//	auto& bone = (*iter).second;
	//	if (bone)
	//	{
	//		bone->update(currentTime);
	//		nodeTransform = bone->getLocalTransform(); // todo verify this behaviour
	//	}
	//}

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

	glm::mat4 globalTransformation = parentTransform * nodeTransform; // transform from local space to model space

	if (m_bones.find(nodeName) != m_bones.end())
	{
		auto bone = m_bones[nodeName];
		bone->update(currentTime);
		nodeTransform = bone->getLocalTransform(); // todo verify this behaviour
		globalTransformation = parentTransform * nodeTransform;
		finalBoneMatrices[nodeName] = globalTransformation;
	}

	for (int i = 0; i < nodeData.childrenCount; i++)
	{
		calculateFinalBoneMatricesHelper(nodeData.children[i], globalTransformation, currentTime, finalBoneMatrices);
	}


}

void Animation::calculateFinalBoneMatrices(float currentTime, std::unordered_map<std::string, glm::mat4>& outFinalBoneMatrices)
{
	return calculateFinalBoneMatricesHelper(m_rootNode, glm::mat4(1.0f), currentTime, outFinalBoneMatrices);
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

#include "AnimationLoader.h"
void Animation::load(UUID uid, const std::string& path)
{
	Engine::get()->getSubSystem<AnimationLoader>()->load(path, Resource<Animation>(uid));

}
