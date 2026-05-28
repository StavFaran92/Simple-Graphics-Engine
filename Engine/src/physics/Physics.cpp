#include "physics/Physics.h"

#include "core/Engine.h"
#include "runtime/Context.h"
#include "runtime/Scene.h"

#include <PxPhysicsAPI.h>

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
