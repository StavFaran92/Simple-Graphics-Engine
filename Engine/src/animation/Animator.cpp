
#include "animation/Animator.h"

#include <cmath>

#include "animation/Animation.h"
#include "animation/AnimationGraph.h"
#include "geometry/Model.h"
#include "runtime/Scene.h"
#include "core/Engine.h"
#include "scripts/ScriptSystem.h"

void Animator::init(SceneResourceRef& scene)
{
	m_animationGraph.setAnimatorOwner(this);
}

void Animator::onStart(Entity e)
{
	m_animationGraph.setDebugOwnerEntity(e.handlerID());
	m_animationGraph.init();
}

void Animator::update(Entity e, float dt)
{
	m_animationGraph.update(dt);

	auto currentAnimation = getCurrentAnimation();
	if (!currentAnimation || currentAnimation->animation.isEmpty() || currentAnimation->animation.resource().isEmpty())
		return;

	auto animResource = currentAnimation->animation.resource();

	if (m_finished)
		return;

	// Increment Animation time
	const float duration = animResource->getDuration();
	m_previousTime = m_currentTime;
	m_currentTime += animResource->getTicksPerSecond() * currentAnimation->playbackSpeed * dt;
	bool wrapped = false;
	if (m_currentTime >= duration)
	{
		if (m_loop)
		{
			m_currentTime = fmod(m_currentTime, duration);
			wrapped = true;
		}
		else
		{
			// Hold the last pose - stay just below duration, the same range looping clips are sampled in
			m_currentTime = std::nextafter(duration, 0.f);
			m_finished = true;
		}
		m_animationGraph.onAnimationEnd();
	}

	// Fire every trigger the playhead crossed this update: [previous, current),
	// split across the end when the clip wrapped, so large dt steps can't skip one
	for (const auto& trigger : currentAnimation->triggers)
	{
		const float frame = (float)trigger.frameID;
		bool crossed = false;
		if (wrapped)
			crossed = frame >= m_previousTime || frame < m_currentTime;
		else if (m_finished)
			crossed = frame >= m_previousTime;
		else
			crossed = frame >= m_previousTime && frame < m_currentTime;

		if (crossed)
			Engine::get()->getSubSystem<ScriptSystem>()->callOnAnimTrigger(e, trigger.name, trigger.frameID);
	}
}

void Animator::getFinalBoneMatrices(const ModelResourceRef meshCollection, std::vector<glm::mat4>& meshSpaceToBoneSpaceBindPoseMat) const
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
	playAnimation(name, true);
}

void Animator::playAnimation(const std::string& name, bool loop)
{
	auto iter = std::find_if(m_animations.begin(), m_animations.end(),
		[&name](const AnimationEntry& e) { return e.name == name; });

	if (iter == m_animations.end())
	{
		logWarning("Animator: animation '{}' not found", name);
		return;
	}

	m_currentAnimIndex = static_cast<int>(std::distance(m_animations.begin(), iter));
	m_currentTime = 0.f;
	m_previousTime = 0.f;
	m_loop = loop;
	m_finished = false;
}

bool Animator::isFinished() const
{
	return m_finished;
}

float Animator::getCurrentFrame() const
{
	return m_currentTime;
}

float Animator::getNormalizedTime() const
{
	auto currentAnimation = getCurrentAnimation();
	if (!currentAnimation || currentAnimation->animation.isEmpty() || currentAnimation->animation.resource().isEmpty())
		return 0.f;

	const float duration = currentAnimation->animation.resource()->getDuration();
	return duration > 0.f ? m_currentTime / duration : 0.f;
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

bool Animator::hasActiveAnimation() const
{
	auto currentAnim = getCurrentAnimation();
	return currentAnim && !currentAnim->animation.isEmpty() ;
}

AnimationGraph& Animator::getAnimationGraph()
{
	return m_animationGraph;
}

std::vector<AssetRef<Asset>> Animator::gatherDependenciesInternal() const
{
	std::vector<AssetRef<Asset>> dependencies;
	for (auto& aEntry : m_animations)
	{
		if(!aEntry.animation.isEmpty())
			dependencies.push_back(aEntry.animation);
	}
	return dependencies;
}
