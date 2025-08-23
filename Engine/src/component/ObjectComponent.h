#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include "runtime/Entity.h"
#include <string>

struct EngineAPI ObjectComponent : public Component
{
        ObjectComponent() = default;
        ObjectComponent(Entity e, const std::string& name) : name(name), e(e) {};

        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);

        template <class Archive>
        void serialize(Archive& archive) {
                SERIALIZED_MEMBER(name);
                SERIALIZED_MEMBER(e);
        }

    std::string name;
    Entity e = Entity::EmptyEntity;
};

REGISTER_COMPONENT(ObjectComponent)
