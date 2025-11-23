#include "render/IRenderer.h"

#include "Graphics.h"
#include "geometry/AABB.h"
#include "memory/BuiltInAssets.h"
#include "component/MaterialComponent.h"
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

	return true;
}
