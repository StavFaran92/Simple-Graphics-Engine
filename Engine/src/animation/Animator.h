#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "animation/Animation.h"
#include "animation/AnimationEntry.h"
#include "animation/AnimationGraph.h"

class EngineAPI Animator : public Component
{
public:
	Animator() = default;

	std::string getName() override { return "Animator"; }

	void resolve(SceneResourceRef& scene) override;

	void onStart();
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
	AnimationGraph& getAnimationGraph();

	template <class Archive>
	void serialize(Archive& archive) {
		SERIALIZED_MEMBER_OPTIONAL(m_currentAnimIndex);
		SERIALIZED_MEMBER_OPTIONAL(m_animations);
		SERIALIZED_MEMBER_OPTIONAL(m_animationGraph);
	}

	float m_currentTime = 0.f;
	std::unordered_map<std::string, glm::mat4> m_finalBoneMatrices;

private:
	int m_currentAnimIndex = 0;
	std::vector<AnimationEntry> m_animations;
	AnimationGraph m_animationGraph;

};

REGISTER_COMPONENT(Animator)
