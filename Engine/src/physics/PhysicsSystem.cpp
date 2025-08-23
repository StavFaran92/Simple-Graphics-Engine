#include "physics/PhysicsSystem.h"

#include "core/Logger.h"

#include "physics/PhysXUtils.h"
#include "component/Component.h"
#include "physics/Physics.h"
#include "runtime/Scene.h"
#include "core/Registry.h"
#include "GL//glew.h"
#include "render/Graphics.h"
#include "render/RenderCommand.h"
#include "systems/BuiltInMeshes.h"
#include <glm/gtx/quaternion.hpp>
#include "component/PlayerControllerComponent.h"

using namespace physx;

#define PX_RELEASE(x)	if(x)	{ x->release(); x = nullptr; }

glm::mat4 PxTransformToMat4(const physx::PxTransform & transform)
{
    glm::quat rotationQuat(transform.q.w, transform.q.x, transform.q.y, transform.q.z);
    glm::vec3 translationVec(transform.p.x, transform.p.y, transform.p.z);
    glm::mat4 mat = glm::translate(glm::mat4(1.0f), translationVec) * glm::toMat4(rotationQuat);

    return mat;
}

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

    m_debugVisualizeShader = Shader::create(SGE_ROOT_DIR + "Resources/Engine/Shaders/UnlitShader.glsl");

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

    auto controllerManager = PxCreateControllerManager(*scene);
    m_CCTControllers[scene] = controllerManager;

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

physx::PxRigidActor* PhysicsSystem::createRigidBody(Transformation& transform, PhysicsComponent& rb)
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

    auto& rBody = e.getComponent<PhysicsComponent>();
    scene->getPhysicsScene()->removeActor(*(physx::PxRigidActor*)rBody.simulatedBody);
}

void PhysicsSystem::createActor(Scene* scene, entt::entity entity)
{
    Entity e{ entity, &scene->getRegistry()};
    auto& transform = e.getComponent<Transformation>();
    auto& rb = e.getComponent<PhysicsComponent>();

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
    auto& rb = e.getComponent<PhysicsComponent>();
    if (rb.collider->getType() != ColliderType::TERRAIN)
    {
        return;
    }
    auto& collider = std::static_pointer_cast<CollisionTerrain>(rb.collider);
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

void PhysicsSystem::createCCTController(Scene* scene, entt::entity entity)
{
    auto iter = m_CCTControllers.find(scene->getPhysicsScene());

    if (iter == m_CCTControllers.end())
    {
        logError("Could not locate scene's CCT Controller manager");
        return;
    }

    auto controllerManager = iter->second;

    float crouchHeight = .25f;

    Entity e(entity, &scene->getRegistry());
    auto pos = e.getComponent<Transformation>().getWorldPosition();

    PxCapsuleControllerDesc desc;

    desc.height = 1.f;
    desc.radius = .3f;
    desc.position = physx::PxExtendedVec3(pos.x, pos.y, pos.z);
    desc.material = m_defaultMaterial;

    //mType = desc.mType;
    //mInitialPosition = desc.mPosition;
    //mStandingSize = height;
    //mCrouchingSize = crouchHeight;
    //mControllerRadius = radius;

    PxController* ctrl = static_cast<PxCapsuleController*>(controllerManager->createController(desc));
    PX_ASSERT(ctrl);

    entity_id * id = new entity_id(e.handlerID());
    ctrl->getActor()->userData = (void*)id;
}

void PhysicsSystem::startScenePhysics(Scene* scene)
{
    for (auto&& [entity, rb] : scene->getRegistry().getRegistry().view<PhysicsComponent>().each())
    {
        createActor(scene, entity);
    }

    for (auto&& [entity, pc] : scene->getRegistry().getRegistry().view<PlayerController>().each())
    {
        createCCTController(scene, entity);
    }
}

void PhysicsSystem::stopScenePhysics(Scene* scene)
{
    for (auto&& [entity, rb] : scene->getRegistry().getRegistry().view<PhysicsComponent>().each())
    {
        removeActor(scene, entity);
    }

    auto iter = m_CCTControllers.find(scene->getPhysicsScene());

    if (iter == m_CCTControllers.end())
    {
        logError("Could not locate scene's CCT Controller manager");
        return;
    }

    iter->second->purgeControllers();
}

void PhysicsSystem::visualizePhysicsShapeDebug(Scene* scene)
{
    auto graphics = Engine::get()->getSubSystem<Graphics>();

    auto physicsScene = scene->getPhysicsScene();

    physx::PxU32 nbActors = physicsScene->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC);
    if (nbActors)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Physics Debug");

        m_debugVisualizeShader->use();

        m_debugVisualizeShader->setViewMatrix(*graphics->view);
        m_debugVisualizeShader->setProjectionMatrix(*graphics->projection);
        m_debugVisualizeShader->setUniformValue("color", glm::vec3(0, 1, 0));

        glDisable(GL_DEPTH_TEST);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0, -1.0);
        glLineWidth(1); // Size in pixels

        std::vector<physx::PxRigidActor*> actors(nbActors);
        physicsScene->getActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC | physx::PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<physx::PxActor**>(&actors[0]), nbActors);

        for (physx::PxRigidActor* actor : actors)
        {
            physx::PxU32 nbShapes = actor->getNbShapes();
            std::vector<physx::PxShape*> shapes(nbShapes);
            actor->getShapes(shapes.data(), nbShapes);

            for (physx::PxShape* shape : shapes)
            {
                physx::PxGeometryHolder geometry = shape->getGeometry();
                physx::PxTransform localPose = shape->getLocalPose();
                physx::PxTransform actorPose = actor->getGlobalPose();
                physx::PxTransform worldPose = actorPose * localPose;
                auto& model = PxTransformToMat4(worldPose);
                

                if (geometry.any().getType() == PxGeometryType::eBOX)
                {
                    PxVec3 extents = geometry.box().halfExtents * 2.;
                    model = glm::scale(model, glm::vec3(extents.x, extents.y, extents.z));
                    m_debugVisualizeShader->setModelMatrix(model);
                    auto& mesh = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::BOX);
                    auto vao = mesh->getPrimaryMesh()->getVAO();
                    RenderCommand::draw(vao);
                }

                if (geometry.any().getType() == PxGeometryType::eSPHERE)
                {
                    float radius2 = geometry.sphere().radius * 2.;
                    model = glm::scale(model, glm::vec3(radius2));
                    m_debugVisualizeShader->setModelMatrix(model);
                    auto& mesh = Engine::get()->getBuiltInMeshes()->getMesh(BuiltInMeshes::MeshType::SPHERE);
                    auto vao = mesh->getPrimaryMesh()->getVAO();
                    RenderCommand::draw(vao);
                }
            }
        }

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);

        glEnable(GL_DEPTH_TEST);

        glPopDebugGroup();
    }
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
    auto& pc = e.getComponent<PhysicsComponent>();
    auto scale = transform.getWorldScale();

    if (pc.colliderType == ColliderType::NONE)
    {
        return;
    }

    if (pc.collider->getType() == ColliderType::BOX)
    {
        auto& collider = std::dynamic_pointer_cast<CollisionBox>(pc.collider);
        shape = createBoxShape(collider->extents.x * scale.x, collider->extents.y * scale.y, collider->extents.z * scale.z);

        if (!shape)
        {
            logError("Failed to create physics shape");
            return;
        }

        Physics::LayerMask mask = collider->layerMask;

        physx::PxFilterData filterData;
        filterData.word0 = mask;

        shape->setQueryFilterData(filterData);
    }
    else if (pc.collider->getType() == ColliderType::SPHERE)
    {
        auto& collider = std::dynamic_pointer_cast<CollisionSphere>(pc.collider);
        if (collider->radius <= 0)
        {
            logWarning("Invalid collider radius: " + std::to_string(collider->radius));
            return;
        }
        shape = createSphereShape(collider->radius * std::max(std::max(scale.x, scale.y), scale.z));

        assert(shape);

        Physics::LayerMask mask = collider->layerMask;

        physx::PxFilterData filterData;
        filterData.word0 = mask;

        shape->setQueryFilterData(filterData);
    }
    else if (pc.collider->getType() == ColliderType::MESH)
    {
        auto& collider = std::dynamic_pointer_cast<CollisionMesh>(pc.collider);
        const std::vector<glm::vec3>& apos = collider->mesh.get()->getPositions();
        shape = createConvexMeshShape(apos);

        Physics::LayerMask mask = collider->layerMask;

        physx::PxFilterData filterData;
        filterData.word0 = mask;

        shape->setQueryFilterData(filterData);
    }
    else if (pc.collider->getType() == ColliderType::TERRAIN)
    {
        auto& collider = std::static_pointer_cast<CollisionTerrain>(pc.collider);
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
            -terrain.getWidth() / 2.0f), PxQuat(PxIdentity));

        body->setGlobalPose(pose);

        PxShape* shape = PxRigidActorExt::createExclusiveShape(*body,
            PxHeightFieldGeometry(heightField, PxMeshGeometryFlags(),
                terrainHeightScale, terrainRowScale, terrainColScale),
            *getDefaultMaterial());

        if (!shape)
        {
            logError("createShape failed!");
            return;
        }
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

                if (e.HasComponent<PhysicsComponent>())
                {
                    auto& rb = e.getComponent<PhysicsComponent>();

                    physx::PxTransform targetPose = actor->getGlobalPose();
                    targetPose.p += physx::PxVec3(rb.m_targetPisition.x, rb.m_targetPisition.y, rb.m_targetPisition.z);
                    targetPose.q = physx::PxQuat(physx::PxIdentity);

                    if (rb.isChanged)
                    {
                        dynamicBody->setKinematicTarget(targetPose);
                        rb.isChanged = false;
                    }
                }

                //if (e.HasComponent<PlayerController>())
                //{
                //    static const PxControllerFilters filters(NULL, NULL, NULL);

                //    auto& pc = e.getComponent<PlayerController>();

                //    auto iter = m_CCTControllers.find(physicsScene);
                //    if (iter != m_CCTControllers.end())
                //    {
                //        auto controllerManager = iter->second;
                //        auto CCTController = controllerManager->getController(pc.controllerIndex);
                //        CCTController->move(physx::PxVec3(pc.disp.x, pc.disp.y, pc.disp.z), 0.0f, deltaTime, filters);
                //    }

                //    //physx::PxTransform targetPose = actor->getGlobalPose();
                //    //targetPose.p += physx::PxVec3(rb.m_targetPisition.x, rb.m_targetPisition.y, rb.m_targetPisition.z);
                //    //targetPose.q = physx::PxQuat(physx::PxIdentity);

                //    //if (rb.isChanged)
                //    //{
                //    //    dynamicBody->setKinematicTarget(targetPose);
                //    //    rb.isChanged = false;
                //    //}
                //}
            }
            else // Dynamic
            {
                entity_id id = *(entity_id*)actor->userData;
                Entity e{ entt::entity(id),  &scene->getRegistry() };
                auto& rb = e.getComponent<PhysicsComponent>();

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

    auto iter = m_CCTControllers.find(physicsScene);
    if (iter != m_CCTControllers.end())
    {
        auto controllerManager = iter->second;
        physx::PxU32 nbControllers = controllerManager->getNbControllers();

        for (physx::PxU32 i = 0; i < nbControllers; ++i)
        {
            physx::PxController* cct = controllerManager->getController(i);
            if (!cct) continue;

            // Get associated actor
            physx::PxRigidActor* actor = cct->getActor();
            if (!actor || !actor->userData) continue;

            // Convert userData to entity
            entity_id id = *(entity_id*)actor->userData;
            Entity e{ entt::entity(id), &scene->getRegistry() };

            // Check for PlayerController component
            if (!e.HasComponent<PlayerController>()) continue;

            auto& pc = e.getComponent<PlayerController>();

            // Move using displacement in component
            static const physx::PxControllerFilters filters(nullptr, nullptr, nullptr);
            cct->move(
                physx::PxVec3(pc.m_disp.x, pc.m_disp.y, pc.m_disp.z),
                0.0f, // min distance
                deltaTime,
                filters
            );
            pc.reset();

            auto& transform = e.getComponent<Transformation>();

            physx::PxTransform pxTransform = actor->getGlobalPose();

            glm::vec3 translation(pxTransform.p.x, pxTransform.p.y, pxTransform.p.z);

            transform.setWorldPosition(translation);
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

            if (e.HasComponent<PhysicsComponent>() && e.getComponent<PhysicsComponent>().colliderType == ColliderType::TERRAIN)
                continue;

            if (e.HasComponent<PlayerController>())
                continue;

            auto& transform = e.getComponent<Transformation>();

            physx::PxTransform pxTransform = actor->getGlobalPose();
            PhysXUtils::fromPhysXTransform(e, pxTransform, transform);
        }
    }
}