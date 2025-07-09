#include "ShapeFactory.h"

#include "core/Engine.h"
#include "Context.h"
#include "Mesh.h"
#include "Entity.h"
#include "Material.h"
#include "Box.h"
#include "Quad.h"
#include "Sphere.h"
#include "Scene.h"
#include "Transformation.h"
#include "Shader.h"
#include "core/Registry.h"
//#include "RenderableComponent.h"
#include "Component.h"
#include "core/CacheSystem.h"
#include "systems/BuiltInMeshes.h"

Entity ShapeFactory::createEntity(SGE_Regsitry* registry, const std::string& name)
{
	auto entity = registry->createEntity(name);
	entity.addComponent<RenderableComponent>();
	entity.addComponent<MaterialComponent>();
	entity.addComponent<MeshComponent>();
	return entity;
}

Entity ShapeFactory::createBoxEntity(SGE_Regsitry* registry)
{
	static int createdBoxCount = 0;
	auto entity = createEntity(registry, "Box_" + std::to_string(createdBoxCount++));
	Resource<MeshCollection> mesh = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::BOX);
	entity.getComponent<MeshComponent>().mesh = mesh;
	return entity;
}

Entity ShapeFactory::createQuad(SGE_Regsitry* registry)
{
	static int createdQuadCount = 0;
	auto entity = createEntity(registry, "Quad_" + std::to_string(createdQuadCount++));
	Resource<MeshCollection> mesh = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::QUAD);
	entity.getComponent<MeshComponent>().mesh = mesh;
	return entity;
}

Entity ShapeFactory::createSphere(SGE_Regsitry* registry)
{
	static int createdSphereCount = 0;
	auto entity = createEntity(registry, "Sphere_" + std::to_string(createdSphereCount++));
	Resource<MeshCollection> mesh = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::SPHERE);
	entity.getComponent<MeshComponent>().mesh = mesh;
	return entity;
}
