#include "ShapeFactory.h"

#include "Engine.h"
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
#include "Registry.h"
//#include "RenderableComponent.h"
#include "Component.h"
#include "CacheSystem.h"

static int createdBoxCount = 0;

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
	
	auto entity = createEntity(registry, "Box_" + std::to_string(createdBoxCount++));


	auto memoryManager = Engine::get()->getMemoryManagementSystem();
	Resource<MeshCollection> mesh = memoryManager->createOrGetCached<MeshCollection>("SGE_BOX_MESH", []() {return Box::createMesh(); });
	entity.getComponent<MeshComponent>().mesh = mesh;
	return entity;
}

void ShapeFactory::createBoxEntity(Entity& e, SGE_Regsitry* registry)
{
	auto memoryManager = Engine::get()->getMemoryManagementSystem();
	Resource<MeshCollection> mesh = memoryManager->createOrGetCached<MeshCollection>("SGE_BOX_MESH", []() {return Box::createMesh(); });
	e.getComponent<MeshComponent>().mesh = mesh;
}

Resource<MeshCollection> ShapeFactory::createBox()
{
	auto memoryManager = Engine::get()->getMemoryManagementSystem();
	return memoryManager->createOrGetCached<MeshCollection>("SGE_BOX_MESH", []() { return Box::createMesh(); });
}

Entity ShapeFactory::createQuad(SGE_Regsitry* registry)
{
	static int createdQuadCount = 0;
	auto entity = createEntity(registry, "Quad_" + std::to_string(createdQuadCount++));
	auto memoryManager = Engine::get()->getMemoryManagementSystem();
	Resource<MeshCollection> mesh = memoryManager->createOrGetCached<MeshCollection>("SGE_QUAD_MESH", []() {return Quad::createMesh(); });
	entity.getComponent<MeshComponent>().mesh = mesh;
	return entity;
}

Entity ShapeFactory::createSphere(SGE_Regsitry* registry)
{
	static int createdSphereCount = 0;
	auto entity = createEntity(registry, "Sphere_" + std::to_string(createdSphereCount++));
	auto memoryManager = Engine::get()->getMemoryManagementSystem();
	Resource<MeshCollection> mesh = memoryManager->createOrGetCached<MeshCollection>("SGE_SPHERE_MESH", []() {return Sphere::createMesh(1, 36, 36); });
	entity.getComponent<MeshComponent>().mesh = mesh;
	return entity;
}
