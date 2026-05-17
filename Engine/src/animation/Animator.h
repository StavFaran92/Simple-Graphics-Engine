#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "animation/Animation.h"

class AnimationGraph;

struct AnimationEntry
{
	std::string name;
	float playbackSpeed = 1.f;
	AnimationAssetRef animation;

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(name);
		SERIALIZED_MEMBER(playbackSpeed);
		SERIALIZED_MEMBER(animation);
	}

};

class EngineAPI Animator : public Component
{
public:
	Animator() = default;

	std::string getName() override { return "Animator"; }

	void update(float dt);
	void getFinalBoneMatrices(const Model* meshCollection, std::vector<glm::mat4>& outFinalBoneMatrices) const;

	void addAnimation(AnimationEntry animation);
	void removeAnimation(const std::string& name);
	void playAnimation(const std::string& name);
	AnimationEntry* getAnimation(const std::string& name);
	AnimationEntry* getAnimation(int index);
	const std::vector<AnimationEntry>& getAllAnimations() const;
	int getCurrentAnimationID() const;
	AnimationEntry* getCurrentAnimation();
	const AnimationEntry* getCurrentAnimation() const;
	std::string getCurrentAnimationName() const;
	bool hasActiveAnimation() const;

	// Animation graph
	void              createAnimationGraph();
	AnimationGraph*   getAnimationGraph()  const { return m_animationGraph.get(); }
	bool              hasAnimationGraph()  const { return m_animationGraph != nullptr; }

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER(m_currentAnimIndex);
		SERIALIZED_MEMBER(m_animations);
	}

	float m_currentTime = 0.f;
	std::unordered_map<std::string, glm::mat4> m_finalBoneMatrices;

private:
	int m_currentAnimIndex = 0;
	std::vector<AnimationEntry> m_animations;
	std::shared_ptr<AnimationGraph> m_animationGraph;

};

REGISTER_COMPONENT(Animator)
