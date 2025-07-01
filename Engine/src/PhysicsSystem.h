#pragma once

#include <vector>
#include <cstdint>
#include "Core.h"
#include <PxPhysicsAPI.h>

#include "Transformation.h"
#include "Configurations.h"
#include <glm/glm.hpp>

class PhysicsSystem
{
public:
	PhysicsSystem() = default;
	~PhysicsSystem() = default;

	bool init();

	physx::PxScene* createScene();

	void startScenePhysics(Scene* scene);
	void stopScenePhysics(Scene* scene);
	void update(Scene* scene, float deltaTime);

	void renderWireframeDebug();

	void close();

private:
	physx::PxRigidActor* createRigidBody(Transformation& trasform, PhysicsComponent& rb);
	void createActor(Scene* scene, entt::entity entity);
	void removeActor(Scene* scene, entt::entity entity);
	void createShape(physx::PxRigidActor* body, Entity e, bool recursive);
	physx::PxShape* createBoxShape(float x, float y, float z);
	physx::PxShape* createSphereShape(float radius);
	physx::PxShape* createConvexMeshShape(const std::vector<glm::vec3>& vertices);
	physx::PxMaterial* getDefaultMaterial() const;
	void createTerrainActor(Scene* scene, entt::entity entity);

private:
	physx::PxDefaultAllocator       m_defaultAllocatorCallback;
	physx::PxDefaultErrorCallback   m_defaultErrorCallback;
	physx::PxDefaultCpuDispatcher* m_dispatcher = nullptr;
	physx::PxTolerancesScale        m_toleranceScale;

	physx::PxFoundation* m_foundation = nullptr;
	physx::PxPhysics* m_physics = nullptr;

	std::vector<physx::PxScene*>    m_scenes;

	physx::PxMaterial* m_defaultMaterial = nullptr;
	physx::PxCooking* m_cooking = nullptr;

	physx::PxPvd* m_pvd = nullptr;

	bool m_isSimulationActive = false;

	bool m_isInit = false;
};