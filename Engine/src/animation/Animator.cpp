
#include "animation/Animator.h"

#include "animation/Animation.h"
#include "geometry/MeshCollection.h"
#include "runtime/Scene.h"

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
	// TODO remove maybe, there is a bug here due to name not being set
	m_currentAnimation = animation;
	m_currentTime = 0.f;
}

void Animator::setPlaybackSpeed(float playbackSpeed)
{
	m_playbackSpeed = playbackSpeed;
}

void Animator::addAnimation(const std::string& name, Resource<Animation> animation)
{
	m_animations[name] = animation;
}

void Animator::removeAnimation(const std::string& name)
{
	auto iter = m_animations.find(name);
	if (iter != m_animations.end())
	{
		m_animations.erase(iter);
	}
}

void Animator::playAnimation(const std::string& name)
{
	Resource<Animation>& anim = getAnimation(name);
	if (!anim.isEmpty())
	{
		playAnimation(anim);
		m_currentAnimationName = name;
	}
}

Resource<Animation> Animator::getAnimation(const std::string& name)
{
	auto iter = m_animations.find(name);
	if (iter == m_animations.end())
	{
		logWarning("Could not find animation: {}", name);
		return Resource<Animation>::empty;
	}

	return iter->second;
}

const std::map<std::string, Resource<Animation>>& Animator::getAllAnimations() const
{
	return m_animations;
}

void Animator::attachToEntity(std::shared_ptr<Component> c, Entity entityHandler, Scene& scene)
{
	(void)scene;
	if (auto ac = std::dynamic_pointer_cast<Animator>(c))
	{
		entityHandler.addComponent<Animator>(*ac);
	}
}
