#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"
#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "animation/Animation.h"

class EngineAPI Animator : public Component
{
public:
	Animator() = default;
	Animator(AssetWrapper<Animation> animation);
	void update(float dt);
	void getFinalBoneMatrices(const MeshCollection* meshCollection, std::vector<glm::mat4>& outFinalBoneMatrices) const;
	void playAnimation(AssetWrapper<Animation> animation);
	void setPlaybackSpeed(float playbackSpeed);

	void addAnimation(const std::string& name, AssetWrapper<Animation> animation);
	void removeAnimation(const std::string& name);
	void playAnimation(const std::string& name);
	AssetWrapper<Animation> getAnimation(const std::string& name);
	const std::map<std::string, AssetWrapper<Animation>>& getAllAnimations() const;
	std::string getCurrentAnimationName() const;

	static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

	template <class Archive>
	void serialize(Archive& archive) {
		archive(m_currentAnimation, m_playbackSpeed, m_animations);
	}

	std::string m_currentAnimationName;
	AssetWrapper<Animation> m_currentAnimation;
	float m_currentTime = 0.f;
	std::unordered_map<std::string, glm::mat4> m_finalBoneMatrices;
	float m_playbackSpeed = 1.f;

private:
	std::map<std::string, AssetWrapper<Animation>> m_animations;

};

REGISTER_COMPONENT(Animator)
