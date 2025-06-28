#include "Animator.h"

#include "Animation.h"
#include "MeshCollection.h"

Animator::Animator(Resource<Animation> animation)
	: m_currentAnimation(animation)
{

}

void Animator::update(float dt)
{
	
	if (!m_currentAnimation.isEmpty())
	{
		// Increment Animation time
		m_currentTime += m_currentAnimation.get()->getTicksPerSecond() * m_playbackSpeed * dt;
		m_currentTime = fmod(m_currentTime, m_currentAnimation.get()->getDuration());
	}
}

void Animator::getFinalBoneMatrices(const MeshCollection* meshCollection, std::vector<glm::mat4>& meshSpaceToBoneSpaceBindPoseMat) const
{
	if (m_currentAnimation.isEmpty()) 
		return;

	std::unordered_map<std::string, glm::mat4> m_intermediateBoneMatrices;
	m_currentAnimation.get()->calculateFinalBoneMatrices(m_currentTime, m_intermediateBoneMatrices);
	
	meshSpaceToBoneSpaceBindPoseMat = meshCollection->getBoneOffsets();

	for (auto& [boneName, boneSpaceToMeshSpaceAnimationPoseMat] : m_intermediateBoneMatrices)
	{
		// Get static bone offset
		auto boneID = meshCollection->getBoneID(boneName);
		if (boneID == -1)
		{
			continue;
		}
		meshSpaceToBoneSpaceBindPoseMat[boneID] = boneSpaceToMeshSpaceAnimationPoseMat * meshSpaceToBoneSpaceBindPoseMat[boneID];
	}
}

void Animator::playAnimation(Resource<Animation> animation)
{
	m_currentAnimation = animation;
	m_currentTime = 0.f;
}

void Animator::setPlaybackSpeed(float playbackSpeed)
{
	m_playbackSpeed = playbackSpeed;
}
