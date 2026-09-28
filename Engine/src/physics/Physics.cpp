#include "physics/Physics.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"

#include <PxPhysicsAPI.h>

#include <algorithm>

using namespace physx;

namespace
{
    struct LayerMaskFilter : public PxQueryFilterCallback
    {
        PxU32 mask;
        LayerMaskFilter(PxU32 m) : mask(m) {}

        PxQueryHitType::Enum preFilter(const PxFilterData&, const PxShape* shape, const PxRigidActor*, PxHitFlags&) override
        {
            PxFilterData shapeData = shape->getQueryFilterData();
            if (mask == 0 || (shapeData.word0 & mask))
                return PxQueryHitType::eBLOCK;
            return PxQueryHitType::eNONE;
        }

        PxQueryHitType::Enum postFilter(const PxFilterData&, const PxQueryHit&) override
        {
            return PxQueryHitType::eBLOCK;
        }
    };

    // Overlaps need touching hits to report more than one result
    struct LayerMaskTouchFilter : public PxQueryFilterCallback
    {
        PxU32 mask;
        LayerMaskTouchFilter(PxU32 m) : mask(m) {}

        PxQueryHitType::Enum preFilter(const PxFilterData&, const PxShape* shape, const PxRigidActor*, PxHitFlags&) override
        {
            PxFilterData shapeData = shape->getQueryFilterData();
            if (mask == 0 || (shapeData.word0 & mask))
                return PxQueryHitType::eTOUCH;
            return PxQueryHitType::eNONE;
        }

        PxQueryHitType::Enum postFilter(const PxFilterData&, const PxQueryHit&) override
        {
            return PxQueryHitType::eTOUCH;
        }
    };
}

bool Physics::raycast(glm::vec3 origin, glm::vec3 dir, float distance, HitResult& hitResult, LayerMask mask)
{
    PxScene* scene = Engine::get()->getContext()->getActiveScene()->getPhysicsScene();
    PxRaycastBuffer hit;

    const PxHitFlags outputFlags = PxHitFlag::ePOSITION | PxHitFlag::eNORMAL;

    PxQueryFilterData filterData;
    filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

    LayerMaskFilter filterCallback(static_cast<PxU32>(mask));

    if (scene->raycast(PxVec3(origin.x, origin.y, origin.z), PxVec3(dir.x, dir.y, dir.z), distance, hit, outputFlags, filterData, &filterCallback))
    {
        entity_id id = *(entity_id*)hit.block.actor->userData;
        hitResult.e = { entt::entity(id), &Engine::get()->getContext()->getActiveScene()->getRegistry() };
        hitResult.position = glm::vec3(hit.block.position.x, hit.block.position.y, hit.block.position.z);
        hitResult.normal = glm::vec3(hit.block.normal.x, hit.block.normal.y, hit.block.normal.z);
        hitResult.distance = hit.block.distance;

        return true;
    }

    return false;
}

std::vector<Entity> Physics::overlapSphere(glm::vec3 center, float radius, LayerMask mask)
{
    std::vector<Entity> result;

    auto activeScene = Engine::get()->getContext()->getActiveScene();
    PxScene* scene = activeScene->getPhysicsScene();

    const PxU32 maxHits = 32;
    PxOverlapHit hitBuffer[maxHits];
    PxOverlapBuffer hit(hitBuffer, maxHits);

    PxQueryFilterData filterData;
    filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;

    LayerMaskTouchFilter filterCallback(static_cast<PxU32>(mask));

    PxTransform pose(PxVec3(center.x, center.y, center.z));
    scene->overlap(PxSphereGeometry(radius), pose, hit, filterData, &filterCallback);

    std::vector<const PxRigidActor*> seenActors;
    for (PxU32 i = 0; i < hit.getNbTouches(); i++)
    {
        const PxRigidActor* actor = hit.getTouch(i).actor;
        if (!actor->userData)
            continue;

        // An actor with several matching shapes is reported once
        if (std::find(seenActors.begin(), seenActors.end(), actor) != seenActors.end())
            continue;
        seenActors.push_back(actor);

        entity_id id = *(entity_id*)actor->userData;
        result.push_back({ entt::entity(id), &activeScene->getRegistry() });
    }

    return result;
}
