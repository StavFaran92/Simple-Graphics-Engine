#include "render/IRenderer.h"

#include "Graphics.h"
#include "geometry/AABB.h"
#include "memory/BuiltInAssets.h"

#include "component/Transformation.h"
#include "component/ObjectComponent.h"
#include "component/MeshRendererComponent.h"
#include "animation/Animator.h"
#include "core/Logger.h"

bool IRenderer::prepareMeshForRender(Mesh* mesh, const Entity& entityHandler)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	auto& meshRenderer = entityHandler.getComponent<MeshRendererComponent>();

	graphics->mesh = mesh;
	auto& transform = entityHandler.getComponent<Transformation>();
	glm::mat4 modelTransform = transform.getWorldTransformation() * mesh->getRestTransform();
	graphics->model = modelTransform;

	AABB& aabb = mesh->getAABB();
	aabb.transform(modelTransform);

	if (!aabb.isOnFrustum(*graphics->frustum))
	{
		return false;
	}

	//DebugHelper::getInstance().drawAABB(aabb);

	auto matIndex = mesh->getMaterialIndex();
	graphics->material = meshRenderer.at(matIndex);

	if (graphics->material.isEmpty())
	{
		graphics->material = BuiltInAssets::getByName<Material>(SGE_MATERIAL_DEFAULT).resource();
	}

	return true;
}

bool IRenderer::prepareEntityForRender(const Entity& entityHandler)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

	// Display name
	std::string name = entityHandler.getComponent<ObjectComponent>().name;
	logTrace("About to render Entity {}", name);

	// Apply animation logic
	auto animator = entityHandler.tryGetComponent<Animator>();
	if (!animator || animator->m_currentAnimation.isEmpty())
	{
		graphics->shader->setUniformValue("isAnimated", false);
	}
	else
	{
		auto& meshRenderer = entityHandler.getComponent<MeshRendererComponent>();

		std::vector<glm::mat4> finalBoneMatrices;
		animator->getFinalBoneMatrices(meshRenderer.mesh.get(), finalBoneMatrices);
		for (int i = 0; i < finalBoneMatrices.size(); ++i)
		{
			graphics->shader->setUniformValue("finalBonesMatrices[" + std::to_string(i) + "]", finalBoneMatrices[i]);
		}

		graphics->shader->setUniformValue("isAnimated", true);
	}

	return true;
}
