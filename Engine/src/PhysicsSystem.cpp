#include "PhysicsSystem.h"

#include "Logger.h"

#include "PhysXUtils.h"
#include "Component.h"
#include "Physics.h"
#include "Scene.h"
#include "Registry.h"

using namespace physx;

#define PX_RELEASE(x)	if(x)	{ x->release(); x = nullptr; }

bool PhysicsSystem::init()
{
    if (m_isInit)
        return false;;

    // init physx
    m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_defaultAllocatorCallback, m_defaultErrorCallback);
    if (!m_foundation)
    {
        logError("PxCreateFoundation failed!");
        return false;
    }

#ifdef SGE_DEBUG
    m_pvd = PxCreatePvd(*m_foundation);
    physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    m_pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
#endif // SGE_DEBUG

    m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, physx::PxTolerancesScale(), true, m_pvd);
    PxInitExtensions(*m_physics, m_pvd);

    // Set up cooking parameters
    physx::PxCookingParams cookingParams(m_physics->getTolerancesScale());

    cookingParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
    cookingParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
    cookingParams.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eWELD_VERTICES;

    m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, m_physics->getFoundation(), cookingParams);
    if (!m_cooking)
    {
        logError("PxCreateCooking failed!");
        return false;
    }

    m_defaultMaterial = m_physics->createMaterial(0.5f, 0.5f, 0.1f);

    m_isInit = true;

    return true;
}

physx::PxScene* PhysicsSystem::createScene()
{
    physx::PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
    sceneDesc.gravity = physx::PxVec3(0.0f, -9.81f, 0.0f);
    m_dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = m_dispatcher;
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    physx::PxScene* scene = m_physics->createScene(sceneDesc);

#ifdef SGE_DEBUG
    physx::PxPvdSceneClient* pvdClient = scene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    scene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
    scene->setVisualizationParameter(physx::PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
#endif // SGE_DEBUG

    return scene;
}

physx::PxMaterial* PhysicsSystem::getDefaultMaterial() const
{
    return m_defaultMaterial;
}

physx::PxRigidActor* PhysicsSystem::createRigidBody(Transformation& transform, RigidBodyComponent& rb)
{
    transform.forceUpdate();
    auto scale = transform.getLocalScale();
    physx::PxTransform pxTransform = PhysXUtils::toPhysXTransform(transform);
    physx::PxRigidActor* body = nullptr;

    if (rb.type == RigidbodyType::Dynamic || rb.type == RigidbodyType::Kinematic)
    {
        body = m_physics->createRigidDynamic(pxTransform);
        auto dynamicBody = static_cast<physx::PxRigidDynamic*>(body);
        dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::Enum::eLOCK_LINEAR_X, rb.isLockedLinearX);
        dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::Enum::eLOCK_LINEAR_Y, rb.isLockedLinearY);
        dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::Enum::eLOCK_LINEAR_Z, rb.isLockedLinearZ);
        dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::Enum::eLOCK_ANGULAR_X, rb.isLockedAngularX);
        dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::Enum::eLOCK_ANGULAR_Y, rb.isLockedAngularY);
        dynamicBody->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::Enum::eLOCK_ANGULAR_Z, rb.isLockedAngularZ);
        dynamicBody->setAngularDamping(0.5f);
        physx::PxRigidBodyExt::updateMassAndInertia(*dynamicBody, rb.mass);

        if (rb.type == RigidbodyType::Kinematic)
        {
            dynamicBody->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
        }
    }
    else if (rb.type == RigidbodyType::Static)
    {
        body = m_physics->createRigidStatic(pxTransform);
    }

    assert(body);

    return body;

}

physx::PxShape* PhysicsSystem::createBoxShape(float x, float y, float z)
{
    return m_physics->createShape(physx::PxBoxGeometry(x, y, z), *m_defaultMaterial);
}

physx::PxShape* PhysicsSystem::createSphereShape(float radius)
{
    return m_physics->createShape(physx::PxSphereGeometry(radius), *m_defaultMaterial);
}

physx::PxShape* PhysicsSystem::createConvexMeshShape(const std::vector<glm::vec3>& vertices)
{
    physx::PxConvexMeshDesc convexDesc;
    convexDesc.points.count = vertices.size();
    convexDesc.points.stride = sizeof(glm::vec3);
    convexDesc.points.data = vertices.data();
    convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

    physx::PxDefaultMemoryOutputStream buf;
    if (!m_cooking->cookConvexMesh(convexDesc, buf))
        return nullptr;

    physx::PxDefaultMemoryInputData id(buf.getData(), buf.getSize());
    physx::PxConvexMesh* convexMesh = m_physics->createConvexMesh(id);

    physx::PxShape* convexShape = m_physics->createShape(physx::PxConvexMeshGeometry(convexMesh), *m_defaultMaterial, true);

    return convexShape;
}

void PhysicsSystem::removeActor(Scene* scene, entt::entity entity)
{
    Entity e{ entity, &scene->getRegistry()};

    auto& rBody = e.getComponent<RigidBodyComponent>();
    scene->getPhysicsScene()->removeActor(*(physx::PxRigidActor*)rBody.simulatedBody);
}

void PhysicsSystem::createActor(Scene* scene, entt::entity entity)
{
    Entity e{ entity, &scene->getRegistry()};
    auto& transform = e.getComponent<Transformation>();
    auto& rb = e.getComponent<RigidBodyComponent>();

    auto body = createRigidBody(transform, rb);
    createShape(body, e, true);

    scene->getPhysicsScene()->addActor(*body);
    entity_id* id = new entity_id(e.handlerID());
    body->userData = (void*)id;
    rb.simulatedBody = (void*)body;
}

void PhysicsSystem::createTerrainActor(Scene* scene, entt::entity entity)
{
    Entity e{ entity, &scene->getRegistry() };
    auto& transform = e.getComponent<Transformation>();
    auto& collider = e.getComponent<CollisionTerrainComponent>();
    auto& terrain = e.getComponent<Terrain>();

    // todo verify all components exists

    transform.forceUpdate();
    auto scale = transform.getLocalScale();
    physx::PxTransform pxTransform = PhysXUtils::toPhysXTransform(transform);
    
    physx::PxHeightFieldDesc heightFieldDesc;

    const auto& heightmapData = terrain.getHeightmap().get()->getData();

    heightFieldDesc.nbColumns = heightmapData.width;
    heightFieldDesc.nbRows = heightmapData.height;
    heightFieldDesc.samples.data = new unsigned int[sizeof(unsigned int) * heightFieldDesc.nbColumns * heightFieldDesc.nbRows];
    heightFieldDesc.samples.stride = sizeof(unsigned int);
    unsigned char* currentByte = (unsigned char*)heightFieldDesc.samples.data;
    for (int row = 0; row < heightFieldDesc.nbRows; row++)
    {
        for (int column = 0; column < heightFieldDesc.nbColumns; column++)
        {
            physx::PxHeightFieldSample* currentSample = (physx::PxHeightFieldSample*)currentByte;

            // we flip the row and col order, I have no idea why physx accept the data like that
            auto a = static_cast<uint8_t*>(heightmapData.data)[(column * heightmapData.width + row) * heightmapData.bpp]; 
            currentSample->height = static_cast<int16_t>(a/* * 2^8*/); // we use the full range of the height map field

            currentSample->clearTessFlag();
            currentByte += heightFieldDesc.samples.stride;
        }
    }
    PxHeightField* heightField = m_cooking->createHeightField(heightFieldDesc, m_physics->getPhysicsInsertionCallback());
    if (!heightField)
    {
        logError("createHeightField failed!");
        return;
    }
    // create shape for heightfield		
    //PxTransform pose(PxVec3(-(heightFieldDesc.nbRows * terrain.getHeight()) / 2.0f,
    //    0.0f,
    //    -((PxReal)heightFieldDesc.nbColumns * terrain.getWidth()) / 2.0f),
    //    PxQuat(PxIdentity));

    float terrainColScale = (float)terrain.getWidth() / (PxReal)heightFieldDesc.nbColumns;
    float terrainRowScale = (float)terrain.getHeight() / (PxReal)heightFieldDesc.nbRows;
    float terrainHeightScale = terrain.getScale() / 255.f;

    PxTransform pose(PxVec3(-terrain.getHeight() / 2.f,
        0.0f, 
        -terrain.getWidth() / 2.0f) , PxQuat(PxIdentity));

    PxRigidActor* heightFieldActor = m_physics->createRigidStatic(pose); // todo fix
    if (!heightFieldActor)
    {
        logError("createRigidStatic failed!");
        return;
    }

    

    PxShape* shape = PxRigidActorExt::createExclusiveShape(*heightFieldActor, 
        PxHeightFieldGeometry(heightField, PxMeshGeometryFlags(), 
            terrainHeightScale, terrainRowScale, terrainColScale),
        *getDefaultMaterial());

    if (!shape)
    {
        logError("createShape failed!");
        return;
    }

    scene->getPhysicsScene()->addActor(*heightFieldActor);
    entity_id* id = new entity_id(e.handlerID());
    heightFieldActor->userData = (void*)id;
}

void PhysicsSystem::startScenePhysics(Scene* scene)
{
    for (auto&& [entity, rb] : scene->getRegistry().getRegistry().view<RigidBodyComponent>().each())
    {
        createActor(scene, entity);
    }

    for (auto&& [entity, rb] : scene->getRegistry().getRegistry().view<CollisionTerrainComponent>().each())
    {
        createTerrainActor(scene, entity);
    }
}

void PhysicsSystem::stopScenePhysics(Scene* scene)
{
    for (auto&& [entity, rb] : scene->getRegistry().getRegistry().view<RigidBodyComponent>().each())
    {
        removeActor(scene, entity);
    }
}

void PhysicsSystem::renderWireframeDebug()
{

}


void PhysicsSystem::close()
{
    if (!m_isInit)
        return;

    PX_RELEASE(m_dispatcher);
    PX_RELEASE(m_physics);
    if (m_pvd)
    {
        physx::PxPvdTransport* transport = m_pvd->getTransport();
        PX_RELEASE(m_pvd);
        PX_RELEASE(transport);
    }
    PX_RELEASE(m_foundation);
}

void PhysicsSystem::createShape(physx::PxRigidActor* body, Entity e, bool recursive)
{
    physx::PxShape* shape = nullptr;
    auto& transform = e.getComponent<Transformation>();
    auto scale = transform.getWorldScale();

    if (e.HasComponent<CollisionBoxComponent>())
    {
        auto& collider = e.getComponent<CollisionBoxComponent>();
        shape = createBoxShape(collider.halfExtent * scale.x, collider.halfExtent * scale.y, collider.halfExtent * scale.z);

        if (!shape)
        {
            logError("Failed to create physics shape");
            return;
        }

        Physics::LayerMask mask = collider.layerMask;

        physx::PxFilterData filterData;
        filterData.word0 = mask;

        shape->setQueryFilterData(filterData);
    }
    else if (e.HasComponent<CollisionSphereComponent>())
    {
        auto& collider = e.getComponent<CollisionSphereComponent>();
        if (collider.radius <= 0)
        {
            logWarning("Invalid collider radius: " + std::to_string(collider.radius));
            return;
        }
        shape = createSphereShape(collider.radius * std::max(std::max(scale.x, scale.y), scale.z));

        assert(shape);

        Physics::LayerMask mask = collider.layerMask;

        physx::PxFilterData filterData;
        filterData.word0 = mask;

        shape->setQueryFilterData(filterData);
    }
    else if (e.HasComponent<CollisionMeshComponent>())
    {
        auto collisionMeshComponent = e.getComponent<CollisionMeshComponent>();
        const std::vector<glm::vec3>& apos = collisionMeshComponent.mesh.get()->getPositions();
        shape = createConvexMeshShape(apos);

        Physics::LayerMask mask = collisionMeshComponent.layerMask;

        physx::PxFilterData filterData;
        filterData.word0 = mask;

        shape->setQueryFilterData(filterData);
    }

    if (shape)
    {

        auto translation = transform.getWorldPosition();
        auto orientation = transform.getWorldRotation();

        physx::PxVec3 pxTranslation(translation.x, translation.y, translation.z);
        pxTranslation -= body->getGlobalPose().p;
        physx::PxQuat pxRotation(orientation.x, orientation.y, orientation.z, orientation.w);
        pxRotation *= body->getGlobalPose().q.getConjugate();

        auto physxTransform = physx::PxTransform(pxTranslation, pxRotation);

        //auto physxTransform = PhysXUtils::toPhysXTransform(transform);
        shape->setLocalPose(physxTransform);
        body->attachShape(*shape);
        shape->release();
    }

    if (recursive)
    {
        for (auto [eid, child] : e.getChildren())
        {
            createShape(body, child, true);
        }
    }
}

void PhysicsSystem::update(Scene* scene, float deltaTime)
{
    auto physicsScene = scene->getPhysicsScene();

    physicsScene->simulate(1 / 120.f);
    physicsScene->fetchResults(true);

    // Update kinematics
    physx::PxU32 nbDynamicActors = physicsScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC);
    if (nbDynamicActors)
    {
        std::vector<physx::PxRigidActor*> actors(nbDynamicActors);
        physicsScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC, reinterpret_cast<physx::PxActor**>(&actors[0]), nbDynamicActors);

        for (physx::PxRigidActor* actor : actors)
        {
            auto dynamicBody = static_cast<physx::PxRigidDynamic*>(actor);
            auto& flags = dynamicBody->getRigidBodyFlags();
            if (flags.isSet(physx::PxRigidBodyFlag::eKINEMATIC))
            {
                entity_id id = *(entity_id*)actor->userData;
                Entity e{ entt::entity(id), &scene->getRegistry()};
                auto& rb = e.getComponent<RigidBodyComponent>();

                physx::PxTransform targetPose = actor->getGlobalPose();
                targetPose.p += physx::PxVec3(rb.m_targetPisition.x, rb.m_targetPisition.y, rb.m_targetPisition.z);
                targetPose.q = physx::PxQuat(physx::PxIdentity);

                if (rb.isChanged)
                {
                    dynamicBody->setKinematicTarget(targetPose);
                    rb.isChanged = false;
                }
            }
            else // Dynamic
            {
                entity_id id = *(entity_id*)actor->userData;
                Entity e{ entt::entity(id),  &scene->getRegistry() };
                auto& rb = e.getComponent<RigidBodyComponent>();

                if (rb.isChanged)
                {

                    physx::PxVec3 force(rb.m_force.x, rb.m_force.y, rb.m_force.z);
                    dynamicBody->addForce(force);
                    rb.isChanged = false;
                    rb.m_force = glm::vec3(0);
                }
            }
        }
    }

    // Retrieve Graphics transform from Physics transform
    physx::PxU32 nbActors = physicsScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC);
    if (nbActors)
    {
        std::vector<physx::PxRigidActor*> actors(nbActors);
        physicsScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<physx::PxActor**>(&actors[0]), nbActors);

        for (physx::PxRigidActor* actor : actors)
        {
            entity_id id = *(entity_id*)actor->userData;
            Entity e{ entt::entity(id),  &scene->getRegistry() };

            if (e.HasComponent<CollisionTerrainComponent>())
                continue;

            auto& transform = e.getComponent<Transformation>();

            physx::PxTransform pxTransform = actor->getGlobalPose();
            PhysXUtils::fromPhysXTransform(e, pxTransform, transform);
        }
    }
}