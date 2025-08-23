#include "geometry/ShapeFactory.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "geometry/Mesh.h"
#include "runtime/Entity.h"
#include "render/Material.h"
#include "geometry/Box.h"
#include "geometry/Quad.h"
#include "geometry/Sphere.h"
#include "runtime/Scene.h"
#include "component/Transformation.h"
#include "render/Shader.h"
#include "core/Registry.h"
//#include "component/RenderableComponent.h"
#include "component/RenderableComponent.h"
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
