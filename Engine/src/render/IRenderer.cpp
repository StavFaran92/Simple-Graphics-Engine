#include "render/IRenderer.h"

#include "Graphics.h"
#include "geometry/AABB.h"
#include "memory/BuiltInAssets.h"
#include "component/MaterialComponent.h"
#include "component/Transformation.h"
#include "component/ObjectComponent.h"
#include "component/MeshComponent.h"
#include "animation/Animator.h"
#include "core/Logger.h"

bool IRenderer::prepareMeshForRender(Mesh* mesh, const Entity& entityHandler)
{
	auto graphics = Engine::get()->getSubSystem<Graphics>();

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
	auto materialComponent = entityHandler.tryGetComponent<MaterialComponent>();

	ResourceWrapper<Material> material;

	if (materialComponent)
	{
		material = materialComponent->at(matIndex);
	}
	else
	{
		material = BuiltInAssets::getByName<Material>(SGE_MATERIAL_DEFAULT).resource();
	}

	graphics->material = material;

	return true;
}
