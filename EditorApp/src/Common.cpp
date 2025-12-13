#include "Common.h"

std::unordered_map<std::string, ResourceWrapper<Texture>> icons;

std::vector<SceneObject> sceneObjects;

void focusOnEntity(Entity e, Entity cameraEntity)
{
	// get camera forward
	auto& camera = cameraEntity.getComponent<CameraComponent>();
	auto front = camera.front;

	// get entity location
	auto& targetTransform = e.getComponent<Transformation>();

	glm::vec3 targetLocation = targetTransform.getWorldPosition() - front * 5.f;

	auto targetMesh = e.tryGetComponent<MeshRendererComponent>();
	if (targetMesh)
	{
		auto& targetAABB = targetMesh->mesh.get()->getPrimaryMesh()->getAABB();

		// set destination to location + forward
		targetLocation = targetTransform.getWorldPosition() - front * 5.f + targetAABB.extents() * .5f;

		// create fake frustum
		Frustum fakeFrustum(targetLocation + front * 10.f, front, camera.up, camera.right, camera.aspect, camera.getFOVYInRadians(), camera.znear, camera.zfar);

		// we start at the target object location and step back until the object AABB is inside the frustum.
		while (!targetAABB.isOnFrustum(fakeFrustum))
		{
			targetLocation -= front;
			fakeFrustum = Frustum(targetLocation + front * 10.f, front, camera.up, camera.right, camera.aspect, camera.getFOVYInRadians(), camera.znear, camera.zfar);
		}
	}

	auto& transform = cameraEntity.getComponent<Transformation>();
	transform.setLocalPosition(targetLocation);
}