#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "geometry/MeshCollection.h"

struct EngineAPI MeshComponent : public Component
{
        MeshComponent(const Resource<MeshCollection>& mesh) : mesh(mesh) {};
        MeshComponent() = default;

        template <class Archive>
        void serialize(Archive& archive) {
                SERIALIZED_MEMBER(mesh);
                SERIALIZED_MEMBER(materialSlot);
        }

        float materialSlot = 0;
        Resource<MeshCollection> mesh = Resource<MeshCollection>::empty;
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};

REGISTER_COMPONENT(MeshComponent)
