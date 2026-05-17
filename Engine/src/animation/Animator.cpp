
#include "animation/Animator.h"

#include "animation/Animation.h"
#include "animation/AnimationGraph.h"
#include "geometry/Model.h"
#include "runtime/Scene.h"

void Animator::update(float dt)
{
	auto currentAnimation = getCurrentAnimation();
	if (!currentAnimation || currentAnimation->animation.isEmpty() || currentAnimation->animation.resource().isEmpty())
		return;
	
	auto animResource = currentAnimation->animation.resource();

	// Increment Animation time
	m_currentTime += animResource->getTicksPerSecond() * currentAnimation->playbackSpeed * dt;
	m_currentTime = fmod(m_currentTime, animResource->getDuration());
}

void Animator::getFinalBoneMatrices(const Model* meshCollection, std::vector<glm::mat4>& meshSpaceToBoneSpaceBindPoseMat) const
{
	auto currentAnimation = getCurrentAnimation();
	if (!currentAnimation || currentAnimation->animation.isEmpty() || currentAnimation->animation.resource().isEmpty())
		return;

	auto animResource = currentAnimation->animation.resource();
	std::unordered_map<std::string, glm::mat4> m_intermediateBoneMatrices;
	animResource->calculateFinalBoneMatrices(m_currentTime, m_intermediateBoneMatrices);
	
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

void Animator::addAnimation(AnimationEntry animation)
{
	m_animations.push_back(animation);
}

void Animator::removeAnimation(const std::string& name)
{
	auto iter = std::find_if(m_animations.begin(), m_animations.end(),
		[&name](const AnimationEntry& e) { return e.name == name; });

	if (iter != m_animations.end())
	{
		int removedIdx = static_cast<int>(std::distance(m_animations.begin(), iter));
		m_animations.erase(iter);

		if (m_currentAnimIndex > removedIdx)
			--m_currentAnimIndex;
		else if (m_currentAnimIndex == removedIdx)
			m_currentAnimIndex = 0;
	}
}

void Animator::playAnimation(const std::string& name)
{
	auto iter = std::find_if(m_animations.begin(), m_animations.end(),
		[&name](const AnimationEntry& e) { return e.name == name; });

	if (iter == m_animations.end())
		return;

	m_currentAnimIndex = static_cast<int>(std::distance(m_animations.begin(), iter));
	m_currentTime = 0.f;
}

AnimationEntry* Animator::getAnimation(const std::string& name)
{
	auto iter = std::find_if(m_animations.begin(), m_animations.end(),
		[&name](const AnimationEntry& e) { return e.name == name; });

	if (iter == m_animations.end())
		return nullptr;

	int animID = static_cast<int>(std::distance(m_animations.begin(), iter));
	return getAnimation(animID);
}

AnimationEntry* Animator::getAnimation(int index)
{
	if (index >= m_animations.size() || index < 0)
	{
		logWarning("Invalid animID: {}", index);
		return nullptr;
	}
	return &m_animations[index];
}

const std::vector<AnimationEntry>& Animator::getAllAnimations() const
{
	return m_animations;
}

int Animator::getCurrentAnimationID() const
{
	return m_currentAnimIndex;
}

AnimationEntry* Animator::getCurrentAnimation()
{
	int animID = getCurrentAnimationID();
	if (animID >= m_animations.size() || animID < 0)
	{
		logWarning("Invalid animID: {}", animID);
		return nullptr;
	}
	return &m_animations[animID];
}

const AnimationEntry* Animator::getCurrentAnimation() const
{
	int animID = getCurrentAnimationID();
	if (animID >= m_animations.size() || animID < 0)
	{
		return nullptr;
	}
	return &m_animations[animID];
}

std::string Animator::getCurrentAnimationName() const
{
	auto anim = getCurrentAnimation();
	if (!anim)
		return "";
	return anim->name;
}

void Animator::createAnimationGraph()
{
	if (!m_animationGraph)
	{
		m_animationGraph = std::make_shared<AnimationGraph>();
		m_animationGraph->setAnimatorOwner(this);
	}
}

bool Animator::hasActiveAnimation() const
{
	auto currentAnim = getCurrentAnimation();
	return currentAnim && !currentAnim->animation.isEmpty() ;
}
