#pragma once
#include "core/Core.h"
#include "serialize/CerealHelpers.h"
#include <memory>

class Scene;
class Entity;

struct EngineAPI Component
{
public:
virtual ~Component() = default;

template <class Archive>
void serialize(Archive& archive) {
}
};

template<typename T>
std::shared_ptr<Component> getComponentIfExists(const Entity& e)
{
std::shared_ptr<Component> c;
if (e.HasComponent<T>())
{
c = std::make_shared<T>(e.getComponent<T>());
}
return c;
}

#define REGISTER_COMPONENT(TYPE) \
CEREAL_REGISTER_TYPE(TYPE); \
CEREAL_REGISTER_POLYMORPHIC_RELATION(Component, TYPE); \
inline ComponentSerializeFnRegister<TYPE> TYPE##_serializeRegister(getComponentIfExists<TYPE>); \
inline ComponentDeserializeFnRegister<TYPE> TYPE##_deserializeRegister(TYPE::attachToEntity);
