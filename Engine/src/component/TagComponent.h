#include "component/Component.h"
#include "component/ComponentSerializer.h"
#include <string>

struct EngineAPI TagComponent : public Component
{
static void attachToEntity(std::shared_ptr<Component>, Entity, Scene&);
std::string tag;
};

REGISTER_COMPONENT(TagComponent)
