#include "animation/AnimationSystem.h"

#include "core/Engine.h"
#include "runtime/Scene.h"
#include "animation/Animator.h"
#include "component/MeshRendererComponent.h"

AnimationSystem::AnimationSystem()
{
	Engine::get()->registerSubSystem<AnimationSystem>(this);
}

void AnimationSystem::update(Scene* scene, float dt)
{
	for (auto&& [e, animator, mesh] : scene->getRegistry().getRegistry().view<Animator, MeshRendererComponent>().each())
	{
		Entity entity(e, &scene->getRegistry());
		animator.update(entity, dt);
	}
}
