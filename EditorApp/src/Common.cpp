#include "Common.h"
#include "EditorCamera.h"
#include "commands/SceneCommands.h"

#include <limits>

std::unordered_map<std::string, TextureResourceRef> icons;

std::vector<SceneObject> sceneObjects;

void openSceneInEditor(const SceneAssetRef& scene)
{
	UUID from = Engine::get()->getContext()->getActiveSceneAsset().getUID();
	UUID to = scene.getUID();
	if (from == to)
		return;

	std::string error;
	if (!CommandStack::Instance().run(std::make_unique<OpenSceneCmd>(from, to), error))
		logError("Opening scene '{}' failed: {}", scene.info().name, error);
}

namespace
{
	// Grows the world box by the entity's mesh bounds (a 1-unit box without a mesh), then by its children's
	void growBounds(Entity e, glm::vec3& min, glm::vec3& max)
	{
		auto& transform = e.getComponent<Transformation>();

		AABB box = AABB::createFromMinMax(glm::vec3(-0.5f), glm::vec3(0.5f));
		if (auto mesh = e.tryGetComponent<MeshRendererComponent>())
		{
			if (auto model = mesh->mesh.resource(); model && model->getPrimaryMesh())
				box = model->getPrimaryMesh()->getAABB();
		}

		box.transform(transform.getWorldTransformation());
		min = glm::min(min, box.getMin());
		max = glm::max(max, box.getMax());

		for (auto& [_, child] : transform.getChildren())
			growBounds(child, min, max);
	}
}

void focusOnEntity(Entity e, Entity cameraEntity, glm::vec3 direction)
{
	glm::vec3 min(std::numeric_limits<float>::max());
	glm::vec3 max(std::numeric_limits<float>::lowest());
	growBounds(e, min, max);

	const glm::vec3 center = (min + max) * 0.5f;
	const float radius = std::max(glm::length(max - min) * 0.5f, 0.1f);

	// Far enough for a sphere around the box to fit the vertical field of view, plus a margin
	auto& camera = cameraEntity.getComponent<CameraComponent>();
	const float distance = radius / std::sin(camera.getFOVYInRadians() * 0.5f) * 1.2f;

	auto* editorCamera = dynamic_cast<EditorCamera*>(cameraEntity.getComponent<NativeScriptComponent>().script.get());
	if (editorCamera)
		editorCamera->lookAt(center - glm::normalize(direction) * distance, center);
}