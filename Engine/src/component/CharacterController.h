#include "component/Component.h"
#include "component/ComponentSerializer.h"

struct CharacterController : public Component
{
        CharacterController() = default;
        static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
};

REGISTER_COMPONENT(CharacterController)
